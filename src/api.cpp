#include <lavi/lang/api.hpp>
#include <lavi/lang/preprocessor.hpp>
#include <lavi/lang/lexer.hpp>
#include <lavi/lang/parser.hpp>
#include <lavi/lang/error.hpp>

#include <andy/file.hpp>

extern void create_builtin_libs();

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

            void contained_class(lavi::lang::interpreter *interpreter, std::shared_ptr<lavi::lang::structure> cls, std::shared_ptr<lavi::lang::structure> contained) {
                auto cls_obj = to_object(interpreter, contained);
                cls->variables[contained->name] = cls_obj;
            }

            std::shared_ptr<lavi::lang::object> call(lavi::lang::interpreter *interpreter, lavi::lang::function_call __call) {
                if(__call.name == "yield") {
                    lavi::lang::lexer lexer("", std::string(__call.name));
                    lexer.tokenize();
                    lavi::lang::parser parser;
                    auto ast = parser.parse_all(lexer);
                    ast = ast.childrens().front();
                    auto ret = interpreter->execute(ast);
                    return ret;
                }

                if(__call.object) {
                    interpreter->push_context_with_object(__call.object);

                    auto method = __call.object->cls->instance_functions.find(__call.name);

                    if(method == __call.object->cls->instance_functions.end()) {
                        throw std::runtime_error("Class " + std::string(__call.object->cls->name) + " does not have an instance function called '" + std::string(__call.name) + "'");
                    }

                    __call.method = method->second.get();
                }

                // std::shared_ptr<lavi::lang::object> ret = interpreter->call(__call);
                lavi::lang::error::internal("Temporary disabled code reached at " + std::string(__FILE__) + ":" + std::to_string(__LINE__));

                interpreter->pop_context();

                std::shared_ptr<lavi::lang::object> ret = nullptr;
                return ret;
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
                } else if(interpreter->current_context->cls)
                {
                    auto run_it = interpreter->current_context->cls->functions.find(function_name);

                    if(run_it != interpreter->current_context->cls->functions.end()) {
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
                        throw std::runtime_error("function '" + std::string(function_name) + "' is not defined in object of type " + std::string(interpreter->current_context->self->cls->name));
                    } else if(interpreter->current_context->cls) {
                        throw std::runtime_error("function '" + std::string(function_name) + "' is not defined in class " + std::string(interpreter->current_context->cls->name));
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
                std::shared_ptr<lavi::lang::structure> cls,
                std::vector<std::shared_ptr<lavi::lang::object>> positional_params,
                std::map<std::string_view, std::shared_ptr<lavi::lang::object>> named_params
            )
            {
                auto obj = object::instantiate(interpreter, cls);

                if(obj->cls->base) {
                    obj->base_instance = object::instantiate(interpreter, obj->cls->base);
                }

                auto init_it = cls->instance_functions.find("init");

                if(init_it != cls->instance_functions.end()) {
                    call(interpreter, "init", obj, std::move(positional_params), std::move(named_params));
                } else {
                    // Default constructor

                    if(obj->cls->base) {
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

                return obj->cls != interpreter->FalseClass && obj->cls != interpreter->NullClass;
            }

            bool is_a(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object> obj, std::shared_ptr<lavi::lang::structure> cls)
            {
                auto current_cls = obj->cls;

                while(current_cls) {
                    if(current_cls == cls) {
                        return true;
                    }
                    current_cls = current_cls->base;
                }

                return false;
            }
            
        };
    }; // namespace lang
};