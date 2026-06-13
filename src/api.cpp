#include <lavi/lang/api.hpp>
#include <lavi/lang/preprocessor.hpp>
#include <lavi/lang/lexer.hpp>
#include <lavi/lang/parser.hpp>
#include <lavi/lang/error.hpp>

#include <andy/file.hpp>

extern void create_builtin_libs();

std::map<std::string_view, lavi::lang::parser::ast_node> node_cache;

lavi::lang::parser::ast_node& parse_and_cache_node(
    lavi::lang::interpreter* interpreter,
    std::string_view source_code
)
{
    if(auto it = node_cache.find(source_code); it != node_cache.end()) {
        return it->second;
    }

    // Yes, the cache is kept alive during the execution of the program
    lavi::lang::lexer* lexer = new lavi::lang::lexer("", std::move(std::string(source_code)));

    lexer->tokenize();

    lavi::lang::preprocessor preprocessor;
    preprocessor.process(source_code, *lexer);

    lavi::lang::parser p;
    auto node = p.parse_all(*lexer);

    return node_cache[source_code] = std::move(node.childrens().front());
}

namespace lavi
{
    namespace lang
    {
        namespace api
        {
            std::shared_ptr<lavi::lang::object> evaluate(lavi::lang::interpreter* interpreter, std::filesystem::path path, int argc, char** argv)
            {
                lavi::lang::parser::ast_node root_node;

                std::string source = lavi::file::read_all_text<char>(path);

                std::string path_str = path.string();
                lavi::lang::lexer l(std::move(path_str), std::move(source));
                l.tokenize();

                {
            
                    lavi::lang::preprocessor preprocessor;
                    preprocessor.process(path_str, l);
            
                    lavi::lang::parser p;
                    root_node = p.parse_all(l);
                }

                create_builtin_libs();
        
                interpreter->main_lexer = &l;

                for(int i = 0; i < argc; i++) {
                    interpreter->args.push_back(argv[i]);
                }

                interpreter->input_file_path = path;
                std::shared_ptr<lavi::lang::object> ret = interpreter->execute_all(root_node);
        
                return ret;
            }

            void contained_class(std::shared_ptr<lavi::lang::klass> klass, std::shared_ptr<lavi::lang::klass> contained) {
                klass->functions[contained->name] = std::make_shared<lavi::lang::function>(contained->name, [contained](lavi::lang::interpreter* interpreter) {
                    auto cls_object = lavi::lang::api::to_object(interpreter, contained);
                    return cls_object;
                });
            }

            std::shared_ptr<lavi::lang::object> call(
                lavi::lang::interpreter* interpreter,
                std::string_view function_name,
                std::shared_ptr<lavi::lang::object> object,
                std::vector<std::shared_ptr<lavi::lang::object>> positional_params,
                std::map<std::string_view, std::shared_ptr<lavi::lang::object>> named_params
            )
            {
                if(object) {
                    interpreter->push_context_with_object(object);
                } else {
                    interpreter->push_context();
                }

                if(positional_params.empty() && named_params.empty()) {
                    auto variable_it = interpreter->current_context->variables.find(function_name);

                    if(variable_it != interpreter->current_context->variables.end()) {
                        interpreter->pop_context();

                        return variable_it->second;
                    }
                }

                interpreter->current_context->positional_params = std::move(positional_params);
                interpreter->current_context->named_params = std::move(named_params);

                std::shared_ptr<lavi::lang::function> function = nullptr;

                auto run_it = interpreter->current_context->functions.find(function_name);

                if(run_it != interpreter->current_context->functions.end())
                {
                    function = run_it->second;
                } else if(interpreter->current_context->klass)
                {
                    auto run_it = interpreter->current_context->klass->functions.find(function_name);

                    if(run_it != interpreter->current_context->klass->functions.end()) {
                        function = run_it->second;
                    }
                }

                if(!function) {
                    auto run_it = interpreter->global_context->functions.find(function_name);

                    if(run_it != interpreter->global_context->functions.end()) {
                        function = run_it->second;
                    }
                }

                if(!function) {
                    if(interpreter->current_context->self) {
                        throw std::runtime_error("function '" + std::string(function_name) + "' is not defined in object of type " + std::string(interpreter->current_context->self->klass->name));
                    } else if(interpreter->current_context->klass) {
                        throw std::runtime_error("function '" + std::string(function_name) + "' is not defined in class " + std::string(interpreter->current_context->klass->name));
                    } else {
                        throw std::runtime_error("function '" + std::string(function_name) + "' is not defined in current context");
                    }
                }

                std::shared_ptr<lavi::lang::object> ret = nullptr;

                if(function->block_ast.childrens().size()) {
                    for(size_t i = 0; i < function->positional_params.size(); i++) {
                        interpreter->current_context->variables[function->positional_params[i].name] = interpreter->current_context->positional_params[i];
                    }

                    for(auto& [name, value] : interpreter->current_context->named_params) {
                        interpreter->current_context->variables[name] = value;
                    }
                    
                    if(function->block_ast.type() == lavi::lang::parser::ast_node_type::ast_node_context) {
                        ret = interpreter->execute_all(function->block_ast);
                    } else {
                        ret = interpreter->execute(*function->block_ast.block());
                    }
                } else if(function->native_function) {
                    ret = function->native_function(interpreter);
                }

                interpreter->pop_context();

                return ret;
            }

            std::shared_ptr<lavi::lang::object> call(
                lavi::lang::interpreter* interpreter,
                std::string_view function_name,
                std::initializer_list<std::shared_ptr<lavi::lang::object>> positional_params,
                std::map<std::string_view, std::shared_ptr<lavi::lang::object>> named_params
            )
            {
                std::shared_ptr<lavi::lang::object> object = nullptr;

                const auto& node = parse_and_cache_node(interpreter, function_name);

                const lavi::lang::parser::ast_node* object_node = node.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_object);

                if(object_node) {
                    object = interpreter->execute(object_node->childrens().front());
                }

                switch(node.type())
                {
                    // declname interpreted as a function call with no parameters
                    case lavi::lang::parser::ast_node_type::ast_node_declname:
                        function_name = node.token().content;
                    break;
                    default:
                        function_name = node.decname();
                    break;
                }

                return call(interpreter, function_name, object, std::move(positional_params), std::move(named_params));
            }

            std::shared_ptr<lavi::lang::object> yield(lavi::lang::interpreter* interpreter, std::vector<std::shared_ptr<lavi::lang::object>> position_params, std::map<std::string, std::shared_ptr<lavi::lang::object>> named_params)
            {
                auto* block = interpreter->current_context->given_block;

                if(!block) {
                    throw std::runtime_error("No block given to yield to");
                }

                auto ctx = std::make_shared<lavi::lang::interpreter_context>();
                ctx->is_block_context = true;
                ctx->lexical_parent = interpreter->current_context->given_block_lexical_context;
                interpreter->stack.push_back(ctx);
                interpreter->update_current_context();

                auto fn_params_definition_node = block->child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_params);

                if(fn_params_definition_node) {
                    for(size_t i = 0; i < position_params.size(); ++i)
                    {
                        if(i >= fn_params_definition_node->childrens().size()) {
                            break;
                        }

                        auto* param_node = &fn_params_definition_node->childrens()[i];

                        interpreter->current_context->variables[param_node->token().content] = position_params[i];
                    }
                }

                std::shared_ptr<lavi::lang::object> ret = interpreter->execute(*block->block());

                interpreter->pop_context();

                return ret;
            }

            std::shared_ptr<lavi::lang::object> new_object(
                lavi::lang::interpreter* interpreter,
                std::shared_ptr<lavi::lang::klass> klass,
                std::vector<std::shared_ptr<lavi::lang::object>> positional_params,
                std::map<std::string_view, std::shared_ptr<lavi::lang::object>> named_params
            )
            {
                auto obj = object::instantiate(interpreter, klass);

                auto current_initialization = obj;

                while(current_initialization->klass->base) {
                    auto base = object::instantiate(interpreter, current_initialization->klass->base);

                    current_initialization->base_instance = base;
                    current_initialization = base;
                }

                auto init_it = klass->instance_functions.find("init");

                if(init_it != klass->instance_functions.end()) {
                    call(interpreter, "init", obj, std::move(positional_params), std::move(named_params));
                } else {
                    // Default constructor

                    if(obj->klass->base) {
                        call(interpreter, "init", obj->base_instance, std::move(positional_params), std::move(named_params));
                    }
                }

                return obj;
            }

            bool is_truthy(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object> obj)
            {
                if(!obj) {
                    return false;
                }

                return obj->klass != lavi::lang::false_class && obj->klass != lavi::lang::null_class;
            }

            bool is_a(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object> obj, std::shared_ptr<lavi::lang::klass> klass)
            {
                auto current_cls = obj->klass;

                while(current_cls) {
                    if(current_cls == klass) {
                        return true;
                    }
                    current_cls = current_cls->base;
                }

                return false;
            }
            
        };
    }; // namespace lang
};