#include <iostream>

#include "andy/console.hpp"

#include "lavi/lang/error.hpp"
#include "lavi/lang/interpreter.hpp"
#include "lavi/lang/extension.hpp"
#include "lavi/lang/lang.hpp"
#include "lavi/lang/api.hpp"
#include "lavi/lang/classes.hpp"
#include "lavi/lang/exception.hpp"

std::shared_ptr<lavi::lang::function> execute_method_definition(const lavi::lang::parser::ast_node& class_child)
{
    std::string_view method_name = class_child.decname();

    auto* params_node = class_child.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_params);

    std::vector<lavi::lang::fn_parameter> positional_params;
    std::vector<lavi::lang::fn_parameter> named_params;

    if(params_node) {
        positional_params.reserve(class_child.childrens().size());
        named_params.reserve(class_child.childrens().size());

        for(auto& param : params_node->childrens()) {
            lavi::lang::fn_parameter fn_param;
            std::vector<lavi::lang::fn_parameter>* where_to_push = nullptr;

            if(param.type() == lavi::lang::parser::ast_node_type::ast_node_pair) {
                auto* declname = param.child_from_type(lavi::lang::parser::ast_node_type::ast_node_declname)->childrens().data();

                fn_param.name = declname->token().content;
                fn_param.named = true;

                where_to_push = &named_params;
            } else {
                fn_param.name = std::string(param.token().content);
                where_to_push = &positional_params;
            }

            if(auto* default_node = param.child_from_type(lavi::lang::parser::ast_node_type::ast_node_valuedecl)) {
                fn_param.default_value_node = default_node->childrens().front();
            }

            where_to_push->push_back(std::move(fn_param));
        }
    }

    auto static_node = class_child.child_from_type(lavi::lang::parser::ast_node_type::ast_node_declstatic);

    auto method_type = static_node ? lavi::lang::function_storage_type::class_function : lavi::lang::function_storage_type::instance_function;

    std::shared_ptr<lavi::lang::function> method = std::make_shared<lavi::lang::function>();
    method->name = method_name;
    method->storage_type = method_type;
    method->positional_params = std::move(positional_params);
    method->named_params = std::move(named_params);
    method->block_ast = class_child;
    return method;
}

lavi::lang::interpreter::interpreter()
{
    init();
}

void lavi::lang::interpreter::load(std::shared_ptr<lavi::lang::klass> klass)
{
    global_context->variables[klass->name] = lavi::lang::api::to_object(this, klass);
}

std::shared_ptr<lavi::lang::klass> lavi::lang::interpreter::find_class(const std::string_view& name)
{
    auto it = global_context->variables.find(name);

    if(it != global_context->variables.end() && it->second->klass == lavi::lang::class_class)
    {
        return it->second->as<std::shared_ptr<lavi::lang::klass>>();
    }

    return nullptr;
}

static bool is_named_param(const lavi::lang::parser::ast_node& param)
{
    return param.type() == lavi::lang::parser::ast_node_type::ast_node_valuedecl && param.childrens().size();
}

static void push_context_from_node_object_if_any(lavi::lang::interpreter* interpreter, const lavi::lang::parser::ast_node& node)
{
    const lavi::lang::parser::ast_node* object_node = node.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_object);

    if(object_node) {
        object_node = object_node->childrens().data();
        auto object = interpreter->execute(*object_node);
        if(object == nullptr) {
            object = lavi::lang::object::instantiate(interpreter, lavi::lang::null_class);
        }
        if(object) {
            interpreter->push_context_with_object(object);
        }
    }
}

static void pop_context_from_node_object_if_any(lavi::lang::interpreter* interpreter, const lavi::lang::parser::ast_node& node)
{
    const lavi::lang::parser::ast_node* object_node = node.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_object);

    if(object_node) {
        interpreter->pop_context();
    }
}

static void push_context_from_node(lavi::lang::interpreter* interpreter, const lavi::lang::parser::ast_node& node)
{
    const lavi::lang::parser::ast_node* object_node = node.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_object);

    if(object_node) {
        object_node = object_node->childrens().data();
        auto object = interpreter->execute(*object_node);
        if(object) {
            interpreter->push_context_with_object(object);
        }
    } else {
        interpreter->push_context();
    }
}

static void pop_context_from_node(lavi::lang::interpreter* interpreter, const lavi::lang::parser::ast_node& node)
{
    interpreter->pop_context();
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_fn_decl(const lavi::lang::parser::ast_node& source_code)
{
    auto method = execute_method_definition(source_code);
    current_context->functions[method->name] = method;
    return nullptr;
}

static std::shared_ptr<lavi::lang::klass> do_execute_classdecl(lavi::lang::interpreter* interpreter, const lavi::lang::parser::ast_node& source_code)
{
    std::string_view class_name = source_code.decname();

    auto klass = interpreter->find_class(class_name);

    if (!klass) {
        for(auto& forward_declared_class : interpreter->current_context->forward_declarations) {
            if(forward_declared_class.first == class_name) {
                klass = forward_declared_class.second;
                break;
            }
        }

        if(!klass) {
            klass = lavi::lang::klass::create(class_name);
        }
    }

    klass->is_defined = true;

    auto baseclass_node = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_classdecl_base);

    if (baseclass_node)
    {
        // auto object = lavi::lang::api::to_object(interpreter, klass);

        // interpreter->push_context_with_object(object);

        auto declname_node = baseclass_node->child_from_type(lavi::lang::parser::ast_node_type::ast_node_declname);
        auto base_class = interpreter->find_class(declname_node->token().content);

        if(!base_class) {
            auto forward_declared_class_it = interpreter->current_context->forward_declarations.find(declname_node->token().content);
            if(forward_declared_class_it != interpreter->current_context->forward_declarations.end()) {
                base_class = forward_declared_class_it->second;
            } else {
                base_class = lavi::lang::klass::create(declname_node->token().content);
                interpreter->current_context->forward_declarations.insert({declname_node->token().content, base_class});
            }
        }

        klass->base = base_class;
        base_class->deriveds.push_back(klass);
    }

    interpreter->push_context_with_object(lavi::lang::api::to_object(interpreter, klass));

    for(const auto& class_child : source_code.context()->childrens()) {
        switch (class_child.type())
        {
        case lavi::lang::parser::ast_node_type::ast_node_fn_decl: {
            auto method = execute_method_definition(class_child);
            if(method->storage_type == lavi::lang::function_storage_type::class_function || source_code.decl_type() == "namespace") {
                klass->functions[method->name] = method;
            } else {
                klass->instance_functions[method->name] = method;
            }
        }
        break;
        case lavi::lang::parser::ast_node_type::ast_node_vardecl: {
            std::string_view var_name = class_child.decname();
            const lavi::lang::parser::ast_node* fn_call = class_child.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_call);
            const lavi::lang::parser::ast_node* fn_params = nullptr;
            
            if(fn_params = fn_call->child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_params)) {
                klass->instance_variables[std::string(var_name)] = fn_params->childrens().front();
            } else {
                klass->instance_variables[std::string(var_name)] = lavi::lang::parser::ast_node(); // undefined node, will be treated as nil
            }
        }
        break;
        case lavi::lang::parser::ast_node_type::ast_node_classdecl: {
            auto child_cls = do_execute_classdecl(interpreter, class_child);
            lavi::lang::api::contained_class(klass, child_cls);
            interpreter->load(child_cls);
        }
        break;
        case lavi::lang::parser::ast_node_type::ast_node_enum: {
            std::string_view enum_name = class_child.decname();
            std::string corrected_enum_name;
            corrected_enum_name.reserve(enum_name.size() + enum_name.size() / 2);
            for(auto c : enum_name) {
                c = std::tolower(c);
                corrected_enum_name.push_back(c);
            }

            klass->instance_variables[corrected_enum_name] = lavi::lang::parser::ast_node(); // undefined node, will be treated as nil

            for(const auto& enum_child : class_child.child_from_type(lavi::lang::parser::ast_node_type::ast_node_arraydecl)->childrens()) {
                std::string_view enum_child_name = enum_child.token().content;
                std::string enum_child_name_question;
                enum_child_name_question.reserve(enum_child_name.size() + 1);
                enum_child_name_question.append(enum_child_name);
                enum_child_name_question.push_back('?');
                klass->instance_functions[enum_child_name_question] = std::make_shared<lavi::lang::function>(enum_child_name_question, [enum_name, enum_child_name, corrected_enum_name](lavi::lang::interpreter* interpreter) {
                    auto it = interpreter->current_context->self->variables.find(corrected_enum_name);
                    if(it == interpreter->current_context->self->variables.end()) {
                        lavi::lang::error::internal("Variable {} not found in object of class {} while evaluating {}?", corrected_enum_name, interpreter->current_context->self->klass->name, enum_child_name);
                        exit(1);
                    }

                    return lavi::lang::api::to_object(interpreter, it->second->as<std::string>() == enum_child_name);
                });
            }
        }
        break;
        default:
            throw std::runtime_error(class_child.token().error_message_at_current_position("unexpected token in class declaration"));
            break;
        }
    }

    interpreter->pop_context();

    return klass;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_classdecl(const lavi::lang::parser::ast_node& source_code)
{
    auto klass = do_execute_classdecl(this, source_code);
    load(klass);
    return nullptr;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_valuedecl(const lavi::lang::parser::ast_node& source_code)
{
    switch(source_code.token().kind)
    {
        case lexer::token_kind::token_boolean: {
            std::shared_ptr<lavi::lang::object> obj = lavi::lang::api::to_object(this, source_code.token().boolean_literal);
            return obj;
        }
        break;
        case lexer::token_kind::token_integer: {
            std::shared_ptr<lavi::lang::object> obj = lavi::lang::api::to_object(this, source_code.token().integer_literal);
            return obj;
        }
        case lexer::token_kind::token_float: {
            std::shared_ptr<lavi::lang::object> obj = lavi::lang::api::to_object(this, source_code.token().float_literal);
            return obj;
        }
        break;
        case lexer::token_kind::token_double: {
            std::shared_ptr<lavi::lang::object> obj = lavi::lang::api::to_object(this, source_code.token().double_literal);
            return obj;
        }
        break;
        case lexer::token_kind::token_string: {
            std::shared_ptr<lavi::lang::object> obj = lavi::lang::api::to_object(this, source_code.token().string_literal);
            return obj;
        }
        break;
        case lexer::token_kind::token_null: {
            std::shared_ptr<lavi::lang::object> obj = lavi::lang::object::instantiate(this, lavi::lang::null_class);
            return obj;
        }
        break;
        default:
            throw std::runtime_error("interpreter: unknown node kind");
        break;
    }
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_fn_call(const lavi::lang::parser::ast_node& source_code)
{
    std::string_view function_name;

    switch(source_code.type())
    {
        // declname interpreted as a function call with no parameters
        case lavi::lang::parser::ast_node_type::ast_node_declname:
            function_name = source_code.token().content;
        break;
        default:
            function_name = source_code.decname();
        break;
    }

    // Capture the immediate calling context (excluding global at index 0) before any pushes.
    // This becomes the lexical_parent for any DO...END block passed to this call, ensuring
    // that blocks close over the scope where they are *written* — whether that is a block
    // context (describe/context/it) or a function context (status_tester).
    std::shared_ptr<interpreter_context> call_site_lexical_ctx = nullptr;
    if(stack.size() >= 2) {
        call_site_lexical_ctx = stack.back();
    }

    std::vector<std::shared_ptr<lavi::lang::object>> positional_params;
    std::map<std::string, std::shared_ptr<lavi::lang::object>, std::less<>> named_params;

    const lavi::lang::parser::ast_node* params_node = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_params);

    if(params_node) {
        for(auto& param : params_node->childrens()) {
            const lavi::lang::parser::ast_node* value_node = &param;
            const lavi::lang::parser::ast_node* name = nullptr;
            if(param.type() == lavi::lang::parser::ast_node_type::ast_node_pair) {
                value_node = param.child_from_type(lavi::lang::parser::ast_node_type::ast_node_valuedecl)->childrens().data();
                name = param.child_from_type(lavi::lang::parser::ast_node_type::ast_node_declname)->childrens().data();
            }

            std::shared_ptr<lavi::lang::object> value = nullptr;

            value = execute(*value_node);

            if(name) {
                named_params[name->token().content] = value;
            } else {
                if(!value) {
                    value = lavi::lang::object::instantiate(this, lavi::lang::null_class);
                }
                positional_params.push_back(value);
            }
        }
    }

    // If the function call has an object (obj.fn()), we need to push the context to search for the
    // function in the object and its class, and then pop it. If we don't have an object we need to
    // search in the current context and then in the global context.
    push_context_from_node_object_if_any(this, source_code);

    if(function_name == "new") {
        auto ret = lavi::lang::api::new_object(
            this,
            current_context->klass,
            positional_params
        );

        pop_context_from_node_object_if_any(this, source_code);

        return ret;
    }

    std::shared_ptr<lavi::lang::object> ret = nullptr;

    std::string_view object_name = "";
    bool is_super = function_name == "super";
    bool is_assignment = function_name == "=";
    lavi::lang::function* method_to_call = nullptr;
    bool calling_function_same_object = false;

    if(is_super) {
        if(!current_context->self->base_instance) {
            throw std::runtime_error("super called but current object has no base class");
        }

        pop_context_from_node_object_if_any(this, source_code);

        return lavi::lang::api::call(
            this,
            "init",
            current_context->self->base_instance,
            positional_params
        );
    }

    if(is_assignment && !current_context->self) {
        pop_context_from_node_object_if_any(this, source_code);
        throw std::runtime_error("assignment operator '=' can only be used with a variable");
    }

    if(is_assignment) {
        if(positional_params.size() != 1) {
            pop_context_from_node_object_if_any(this, source_code);
            throw std::runtime_error("assignment operator '=' requires exactly one parameter");
        }
        if(positional_params[0] == nullptr) {
            *current_context->self = std::move(*lavi::lang::object::instantiate(this, lavi::lang::null_class).get());
        } else {
            // If the parameter is only used once (on the right side of the assignment only),
            // we can move it instead of copying it.
            auto use_count = positional_params[0].use_count();
            if(use_count == 1) {
                *current_context->self = std::move(*positional_params[0].get());
            } else {
                auto copy = positional_params[0]->native_copy();
                if(!copy) {
                    pop_context_from_node_object_if_any(this, source_code);
                    throw std::runtime_error("object of class " + std::string(positional_params[0]->klass->name) + " cannot be assigned because it does not support copying");
                }
                *current_context->self = std::move(*copy.get());
            }

        }

        pop_context_from_node_object_if_any(this, source_code);

        return current_context->self;
    }

    auto it = current_context->functions.find(function_name);

    if(it != current_context->functions.end()) {
        method_to_call = it->second.get();
        if(current_context->self) {
            // If the method is found in the current context's functions, it means we are
            // calling a function in the same object, so we set the flag to true to set
            // the current_context->self as the self for the called function.
            calling_function_same_object = true;
        }
    } else if(current_context->self && current_context->self->base_instance) {
        auto it = current_context->self->base_instance->klass->instance_functions.find(function_name);

        if(it != current_context->self->base_instance->klass->instance_functions.end()) {
            method_to_call = it->second.get();
        }
    }
    else if(current_context->klass) {
        auto it = current_context->klass->functions.find(function_name);

        if(it != current_context->klass->functions.end()) {
            method_to_call = it->second.get();
        }
    }

    // Search the lexical_parent chain for the function if not found yet.
    if(!method_to_call) {
        for(auto ctx = current_context->lexical_parent; ctx != nullptr; ctx = ctx->lexical_parent) {
            auto it = ctx->functions.find(function_name);
            if(it != ctx->functions.end()) {
                method_to_call = it->second.get();
                break;
            }
        }
    }

    if(!method_to_call) {
        auto it = global_context->functions.find(function_name);

        if(it != global_context->functions.end()) {
            method_to_call = it->second.get();
        }

        if(!method_to_call) {
            // Last chance
            for(auto ctx = current_context; ctx != nullptr; ctx = ctx->lexical_parent) {
                auto it = ctx->functions.find("missing");
                if(it != ctx->functions.end()) {
                    method_to_call = it->second.get();
                    break;
                }
            }

            if(!method_to_call && current_context->klass) {
                auto it = current_context->klass->functions.find("missing");
                if(it != current_context->klass->functions.end()) {
                    method_to_call = it->second.get();
                }
            }

            if(!method_to_call) {
                auto it = global_context->functions.find("missing");
                if(it != global_context->functions.end()) {
                    method_to_call = it->second.get();
                }
            }

            if(!method_to_call) {
                std::string what;
                what.reserve(100);

                if(current_context->self) {
                    std::string to_string = lavi::lang::api::call(this, "to_string", current_context->self)->as<std::string>();

                    to_string.reserve(to_string.size() + 10 + current_context->self->klass->name.size());
                    to_string.insert(to_string.begin(), '"');
                    to_string.append("\":");
                    to_string.append(current_context->self->klass->name);

                    what = "undefined function '" + std::string(function_name) + "' for " + to_string;

                    to_string.clear();
                } else if(current_context->klass) {
                    what = "undefined function '" + std::string(function_name) + "' for class '" + std::string(current_context->klass->name) + "'";
                } else {
                    what = "undefined function '" + std::string(function_name) + "'";
                }

                auto extension = lavi::lang::extension::which_defines(function_name);

                if(extension) {
                    what.reserve(what.size() + extension->name().size() + 30);
                    what.append(". Did you forget to import '" + extension->name() + "'?");
                }
            
                throw lavi::lang::exception(
                    this,
                    what,
                    lavi::lang::no_function_error_class
                );
            }

            auto previous_positional_params = std::move(positional_params);
            positional_params = std::vector<std::shared_ptr<lavi::lang::object>>();

            auto function_name_obj = lavi::lang::api::to_object(this, function_name);

            positional_params.push_back(function_name_obj);
            positional_params.push_back(lavi::lang::api::to_object(this, std::move(previous_positional_params)));
            positional_params.push_back(lavi::lang::api::to_object(this, std::move(named_params)));
        }
    }

    // If the function call has an object (obj.fn()), we have already pushed the context to search for
    // the function in the object and its class, so we don't need to push it again. If we don't have an
    // object we need to push the context to execute the function in it's own context.
    if(!source_code.fn_object()) {
        if(calling_function_same_object) {
            push_context_with_object(current_context->self->shared_from_this());
        } else {
            // Creates a clean new context
            push_context();
        }
    } else {
        // Already in a clean context pushed with push_context_from_node_object_if_any, so do nothing.
    }

    current_context->caller_node = &source_code;
    // Stores positional_params and named_params on the context so that they can be accessed by the called function and also by execute_yield.
    current_context->positional_params = std::move(positional_params);
    current_context->named_params = std::move(named_params);

    // Store the call-site lexical context on the function's execution context so that
    // execute_yield can use it as the lexical_parent for the DO...END block.
    current_context->given_block_lexical_context = call_site_lexical_ctx;

    if(method_to_call) {
        if(current_context->positional_params.size() < method_to_call->positional_params.size()) {
            for(size_t i = current_context->positional_params.size(); i < method_to_call->positional_params.size(); i++) {
                if(!method_to_call->positional_params[i].default_value_node.is_undefined()) {
                    current_context->positional_params.push_back(execute(method_to_call->positional_params[i].default_value_node.childrens().front()));
                    continue;
                }
                // Found 1 or more missing positional parameters without default values, throw an error.
                break;
            }
        }

        if(current_context->positional_params.size() != method_to_call->positional_params.size()) {
            throw std::runtime_error("function " + std::string(method_to_call->name) + " expects " + std::to_string(method_to_call->positional_params.size()) + " parameters, but " + std::to_string(current_context->positional_params.size()) + " were given");
        }

        for(const auto& param : method_to_call->named_params) {
            auto it = current_context->named_params.find(param.name);

            if(it == current_context->named_params.end()) {
                if(!param.default_value_node.is_undefined()) {
                    current_context->named_params[param.name] = node_to_object(
                        param.default_value_node,
                        current_context->klass,
                        current_context->self ? current_context->self->shared_from_this() : nullptr
                    );
                    continue;
                }
                throw std::runtime_error("function '" + std::string(method_to_call->name) + "' called without named parameter " + param.name);
            }
        }
    }

    current_context->given_block = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_yield_block);

    if(!method_to_call) {
        lavi::lang::error::internal("function '{}' not found, but the execution have continued", function_name);
        exit(1);
    }

    if(method_to_call->block_ast.childrens().size()) {
        for(size_t i = 0; i < method_to_call->positional_params.size(); i++) {
            current_context->variables[method_to_call->positional_params[i].name] = current_context->positional_params[i];
        }

        for(auto& [name, value] : current_context->named_params) {
            current_context->variables[name] = value;
        }
        
        if(method_to_call->block_ast.type() == lavi::lang::parser::ast_node_type::ast_node_context) {
            ret = execute_all(method_to_call->block_ast);
        } else {
            ret = execute(*method_to_call->block_ast.block());
        }
    } else if(method_to_call->native_function) {
        ret = method_to_call->native_function(this);
    }

    for (auto& [name, value] : current_context->variables) {
        if(value && value->base_instance) {
            if(value.use_count() == 2) {
                // used by base_instance and current_context->variables
                value->base_instance = nullptr;
                // Now the use count is 1, it will be destroyed when the current_context is destroyed
            }
        }
    }

    // Todo: Use the current_context.return_value and current_context.has_returned to handle returns

    pop_context();

    return ret;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_arraydecl(const lavi::lang::parser::ast_node& source_code)
{
    std::vector<std::shared_ptr<lavi::lang::object>> array;
    array.reserve(source_code.childrens().size());

    for(auto& child : source_code.childrens()) {
        array.push_back(node_to_object(child));
    }

    return lavi::lang::object::instantiate(this, lavi::lang::array_class, std::move(array));
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_hashdecl(const lavi::lang::parser::ast_node& source_code)
{
    lavi::lang::hash hash(this);

    for(auto& child : source_code.childrens()) {
        auto key_node = child.child_from_type(lavi::lang::parser::ast_node_type::ast_node_declname);
        auto value_node = child.child_from_type(lavi::lang::parser::ast_node_type::ast_node_valuedecl);

        std::shared_ptr<lavi::lang::object> key = node_to_object(key_node->childrens().front());
        std::shared_ptr<lavi::lang::object> value = node_to_object(value_node->childrens().front());

        hash.set(std::move(key), std::move(value));
    }

    return lavi::lang::object::instantiate(this, lavi::lang::hash_class, std::move(hash));
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_interpolated_string(const lavi::lang::parser::ast_node& source_code) 
{
    std::string str;
    for(size_t i = 0; i < source_code.childrens().size(); i++) {
        auto& node_child = source_code.childrens()[i];
        if(node_child.token().type == lavi::lang::lexer::token_type::token_literal)
        {
            switch (node_child.token().kind)
            {
            case lexer::token_kind::token_string:
                str += node_child.token().string_literal;
                break;
            case lexer::token_kind::token_integer:
                str += std::to_string(node_child.token().integer_literal);
                break;
            case lexer::token_kind::token_float:
                str += std::to_string(node_child.token().float_literal);
                break;
            case lexer::token_kind::token_double:
                str += std::to_string(node_child.token().double_literal);
                break;
            case lexer::token_kind::token_boolean:
                str += node_child.token().boolean_literal ? "true" : "false";
                break;
            case lexer::token_kind::token_null:
                str += "null";
                break;
            default:
                node_child.token().error_message_at_current_position("interpreter: unknown token kind");
                break;
            }
        } else {
            std::shared_ptr<lavi::lang::object> obj = execute(node_child);
            if(obj->klass != lavi::lang::string_class) {
                obj = lavi::lang::api::call(this, "to_string", obj);
            }
            str += obj->as<std::string>();
        }
    }
    return lavi::lang::object::create(this, lavi::lang::string_class, std::move(str));
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_vardecl(const lavi::lang::parser::ast_node& source_code)
{
    std::string_view var_name = source_code.decname();
    std::shared_ptr<lavi::lang::object> value = std::make_shared<lavi::lang::object>(lavi::lang::null_class);
    current_context->variables[std::string(var_name)] = value;

    if(auto fn_call = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_call)) {
        value = execute(*fn_call);
    }

    return value;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_conditional(const lavi::lang::parser::ast_node& source_code)
{
    std::shared_ptr<lavi::lang::object> ret = execute(*source_code.condition());
    bool match_codition = source_code.decl_type() == "if";
    bool truthy = lavi::lang::api::is_truthy(this, ret);

    if(truthy == match_codition) {
        auto context = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_context);

        ret = execute(*context);
    } else {
        auto e = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_else);
        if(!e) {
            e = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_conditional);
        }
        if(e) {
            ret = execute(*e);
        }
    }

    return ret;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_while(const lavi::lang::parser::ast_node& source_code)
{
    bool match_condition = source_code.decl_type() == "until";

    while(lavi::lang::api::is_truthy(this, execute(*source_code.condition())) != match_condition) {
        if(current_context->self) {
            push_context_with_object(current_context->self->shared_from_this());
        } else {
            push_block_context();
        }
        execute(*source_code.context());

        if(current_context->has_returned) {
            return current_context->return_value;
        }
        pop_context();
    }

    return nullptr;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_break(const lavi::lang::parser::ast_node& source_code)
{
        current_context->has_returned = true;
        return nullptr;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_context(const lavi::lang::parser::ast_node& source_code)
{
    if(source_code.childrens().size() == 0) {
        return nullptr;
    }

    if(source_code.childrens().size() > 1) {
        if(source_code.childrens().front().type() == lavi::lang::parser::ast_node_type::ast_node_fn_object) {
            auto* fn_object = source_code.childrens().data();
            std::shared_ptr<lavi::lang::object> context_object = node_to_object(
                fn_object->childrens().front(),
                current_context->self ? current_context->self->klass : nullptr,
                current_context->self ? current_context->self->shared_from_this() : nullptr
            );
            push_context_with_object(context_object);
            auto ret = execute_all(source_code.childrens().begin() + 1, source_code.childrens().end());
            pop_context();
            return ret;
        }
    }

    return execute_all(source_code);
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_condition(const lavi::lang::parser::ast_node& source_code)
{
    return node_to_object(source_code.childrens().front());
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_fn_return(const lavi::lang::parser::ast_node& source_code)
{
        if(source_code.childrens().size()) {
            return node_to_object(
                source_code.childrens().front(),
                current_context->self ? current_context->self->klass : nullptr,
                current_context->self ? current_context->self->shared_from_this() : nullptr
            );
        } else {
            return std::make_shared<lavi::lang::object>(lavi::lang::null_class);
        }
        return nullptr;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_for(const lavi::lang::parser::ast_node& source_code)
{
    auto* valuedecl = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_valuedecl);
    if(!valuedecl) {
        valuedecl = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_declname);
    }

    std::shared_ptr<lavi::lang::object> max_object = execute(*valuedecl);

    if(!max_object || max_object->klass != lavi::lang::integer_class) {
        throw std::runtime_error("Cannot iterate over a non-integer value");
    }

    int max = max_object->as<int>();
    int current = 0;

    while(current < max) {
        push_block_context();
        execute_all(*source_code.context());
        pop_context();
        current++;
    }

    return nullptr;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_yield(const lavi::lang::parser::ast_node& source_code)
{
    // Previous block
    auto block = current_context->given_block;

    if(!block) {
        throw lavi::lang::exception(
            this,
            "No block given to yield",
            lavi::lang::runtime_error_class
        );
    }

    // Create a block context whose lexical_parent is the context where the DO...END block
    // was written (captured at call time), not where yield is being executed.
    auto ctx = std::make_shared<interpreter_context>();
    ctx->is_block_context = true;
    ctx->lexical_parent = current_context->given_block_lexical_context;
    stack.push_back(ctx);
    update_current_context();

    auto* fn_params_definition_node = block->child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_params);

    if(fn_params_definition_node) {
        auto* fn_params_call_node = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_params);

        for(size_t i = 0; i < fn_params_definition_node->childrens().size(); i++) {
            auto& param_definition = fn_params_definition_node->childrens()[i];
            std::shared_ptr<lavi::lang::object> value = nullptr;

            if(fn_params_call_node && i < fn_params_call_node->childrens().size()) {
                auto& param_call = fn_params_call_node->childrens()[i];
                value = execute(param_call);
            } else {
                value = std::make_shared<lavi::lang::object>(lavi::lang::null_class);
            }

            current_context->variables[param_definition.token().content] = value;
        }
    }

    std::shared_ptr<lavi::lang::object> ret = execute(*block->block());
    pop_context();
    return ret;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_declname(const lavi::lang::parser::ast_node& source_code)
{
    std::string_view name = source_code.token().content;

    push_context_from_node_object_if_any(this, source_code);

    auto try_find_in_context = [&](const std::shared_ptr<interpreter_context>& ctx) -> std::shared_ptr<lavi::lang::object> {
        auto variable_it = ctx->variables.find(name);
        if(variable_it != ctx->variables.end()) {
            return variable_it->second;
        }
        if(ctx->self && ctx->self->base_instance) {
            auto base_instance_it = ctx->self->base_instance->variables.find(name);
            if(base_instance_it != ctx->self->base_instance->variables.end()) {
                return base_instance_it->second;
            }
        }
        return nullptr;
    };

    std::shared_ptr<lavi::lang::object> ret = nullptr;

    // Walk the lexical_parent chain (starting from the current context) to find the variable.
    for(auto ctx = current_context; ctx != nullptr; ctx = ctx->lexical_parent) {
        ret = try_find_in_context(ctx);
        if(ret != nullptr) {
            break;
        }
    }

    // Always check the global context as a fallback.
    if(ret == nullptr && current_context != global_context) {
        ret = try_find_in_context(global_context);
    }

    // Fallback to a function call

    pop_context_from_node_object_if_any(this, source_code);

    if(ret != nullptr) {
        return ret;
    }

    return execute_fn_call(source_code);

    throw std::runtime_error("'" + std::string(name) + "' is undefined");
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_else(const lavi::lang::parser::ast_node& source_code)
{
    auto context = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_context);
    return execute_all(*context);
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_try(const lavi::lang::parser::ast_node& source_code)
{
    push_block_context();

    std::map<std::string_view, const lavi::lang::parser::ast_node*> catchers;

    for(const auto& child : source_code.childrens()) {
        if(child.type() != lavi::lang::parser::ast_node_type::ast_node_catch) {
            continue;
        }
        auto dectype_node = child.child_from_type(lavi::lang::parser::ast_node_type::ast_node_decltype);
        if(!dectype_node) {
            catchers["var"] = &child;
        } else {
            std::string_view exception_type = child.decl_type();
            catchers[exception_type] = &child;
        }
    }

    try {
        current_context->catching_exception = true;

        auto context = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_context);
        execute(*context);
    } catch(const lavi::lang::exception& e) {
        // Go back to the push_context on the beginning of this function
        while(current_context && !current_context->catching_exception) {
            pop_context();
        }

        current_context->catching_exception = false;

        auto catcher = catchers.find(e.exception_object->klass->name);
        if(catcher == catchers.end()) {
            // If we don't have a catcher for the exception type, we have to throw it again to be caught by an outer
            // try...catch or to terminate the program if it's uncaught.

            // But... If we have a catcher for the "variable" decltype, we can use it to catch any exception
            auto variable_catcher = catchers.find("var");
            if(variable_catcher != catchers.end()) {
                catcher = variable_catcher;
            } else {
                throw;
            }
        }
        auto catch_context = catcher->second->child_from_type(lavi::lang::parser::ast_node_type::ast_node_context);
        push_block_context();
        current_context->variables[std::string(catcher->second->decname())] = e.exception_object;
        execute(*catch_context);
        pop_context();
    }

    pop_context();

    return nullptr;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_throw(const lavi::lang::parser::ast_node& source_code)
{
    std::shared_ptr<lavi::lang::object> exception_object = execute(source_code.childrens().front());

    if(!lavi::lang::api::is_a(this, exception_object, lavi::lang::exception_class)) {
        std::shared_ptr<lavi::lang::object> exception_object_as_string = lavi::lang::api::call(this, "to_string", exception_object);
        exception_object = lavi::lang::api::new_object(
            this,
            lavi::lang::exception_class,
            { exception_object_as_string }
        );
    }

    throw lavi::lang::exception(this, exception_object);

    return exception_object;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute(const lavi::lang::parser::ast_node& source_code)
{
    static auto executors = std::map<lavi::lang::parser::ast_node_type, std::shared_ptr<lavi::lang::object>(lavi::lang::interpreter::*)(const lavi::lang::parser::ast_node&)>{
        { lavi::lang::parser::ast_node_type::ast_node_classdecl,           &lavi::lang::interpreter::execute_classdecl           },
        { lavi::lang::parser::ast_node_type::ast_node_context,             &lavi::lang::interpreter::execute_context             },
        { lavi::lang::parser::ast_node_type::ast_node_fn_return,           &lavi::lang::interpreter::execute_fn_return           },
        { lavi::lang::parser::ast_node_type::ast_node_fn_decl,             &lavi::lang::interpreter::execute_fn_decl             },
        { lavi::lang::parser::ast_node_type::ast_node_valuedecl,           &lavi::lang::interpreter::execute_valuedecl           },
        { lavi::lang::parser::ast_node_type::ast_node_fn_call,             &lavi::lang::interpreter::execute_fn_call             },
        { lavi::lang::parser::ast_node_type::ast_node_interpolated_string, &lavi::lang::interpreter::execute_interpolated_string },
        { lavi::lang::parser::ast_node_type::ast_node_arraydecl,           &lavi::lang::interpreter::execute_arraydecl           },
        { lavi::lang::parser::ast_node_type::ast_node_hashdecl,            &lavi::lang::interpreter::execute_hashdecl            },
        { lavi::lang::parser::ast_node_type::ast_node_vardecl,             &lavi::lang::interpreter::execute_vardecl             },
        { lavi::lang::parser::ast_node_type::ast_node_declname,            &lavi::lang::interpreter::execute_declname            },
        { lavi::lang::parser::ast_node_type::ast_node_conditional,         &lavi::lang::interpreter::execute_conditional         },
        { lavi::lang::parser::ast_node_type::ast_node_while,               &lavi::lang::interpreter::execute_while               },
        { lavi::lang::parser::ast_node_type::ast_node_for,                 &lavi::lang::interpreter::execute_for                 },
        { lavi::lang::parser::ast_node_type::ast_node_break,               &lavi::lang::interpreter::execute_break               },
        { lavi::lang::parser::ast_node_type::ast_node_condition,           &lavi::lang::interpreter::execute_condition           },
        { lavi::lang::parser::ast_node_type::ast_node_else,                &lavi::lang::interpreter::execute_else                },
        { lavi::lang::parser::ast_node_type::ast_node_yield,               &lavi::lang::interpreter::execute_yield               },
        { lavi::lang::parser::ast_node_type::ast_node_try,                 &lavi::lang::interpreter::execute_try                 },
        { lavi::lang::parser::ast_node_type::ast_node_throw,               &lavi::lang::interpreter::execute_throw               }
    };

    auto it = executors.find(source_code.type());

    if(it == executors.end()) {
        lavi::lang::error::internal("No executor found for node type " + std::to_string(static_cast<int>(source_code.type())));
    }

    size_t context_stack_size_before = stack.size();

    auto ret = (this->*it->second)(source_code);

    if(stack.size() != context_stack_size_before && source_code.type() != lavi::lang::parser::ast_node_type::ast_node_try) {
        lavi::lang::error::internal("Node of type '{}' corrupted the context stack by pushing and popping an inconsistent number of contexts", (int)source_code.type());
        exit(1);
    }

    return ret;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_all(
    std::vector<lavi::lang::parser::ast_node>::const_iterator begin,
    std::vector<lavi::lang::parser::ast_node>::const_iterator end
)
{
    std::shared_ptr<lavi::lang::object> result = nullptr;

    for(auto it = begin; it != end; it++) {
        const lavi::lang::parser::ast_node& node = *it;

        if(node.type() == lavi::lang::parser::ast_node_type::ast_node_undefined && node.token().type == lavi::lang::lexer::token_type::token_eof) {
            break;
        }

        result = execute(node);

        if(it->type() == lavi::lang::parser::ast_node_type::ast_node_fn_return) {
            current_context->has_returned = true;
            current_context->return_value = result;
            return result;
        } else if(current_context->has_returned) {
            return current_context->return_value;
        }
    }

    return result;
}

std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::execute_all(const lavi::lang::parser::ast_node& source_code)
{
    return execute_all(source_code.childrens().begin(), source_code.childrens().end());
}

void lavi::lang::interpreter::init()
{
    // The global context. It will not be popped until the end of the program.
    push_context();

    if(!lavi::lang::classes_created) {
        // The first initialization of an interpreter creates all classes
        lavi::lang::klass::create_builtin_classes();
        lavi::lang::classes_created = true;
    }

    for(const auto& klass : lavi::lang::builtin_classes) {
        load(klass);
    }

    for(const auto& variable : lavi::lang::std_class->variables) {
        global_context->variables[variable.first] = variable.second;
    }

    for(const auto& function : lavi::lang::std_class->functions) {
        global_context->functions[function.first] = function.second;
    }
}

const std::shared_ptr<lavi::lang::object> lavi::lang::interpreter::node_to_object(const lavi::lang::parser::ast_node& node, std::shared_ptr<lavi::lang::klass> klass, std::shared_ptr<lavi::lang::object> object)
{
    if(node.token().type == lavi::lang::lexer::token_type::token_literal) {
        return execute(node);
    } else if(node.type() == lavi::lang::parser::ast_node_type::ast_node_fn_call) {
        return execute(node);
    } else if(node.type() == lavi::lang::parser::ast_node_type::ast_node_declname || node.type() == lavi::lang::parser::ast_node_type::ast_node_valuedecl) {
        return execute(node);
    } else if(node.type() == lavi::lang::parser::ast_node_type::ast_node_arraydecl) {
        // Logic moved to execute_arraydecl to support array literals in more places
        return execute(node);
    }
    else if(node.type() == lavi::lang::parser::ast_node_type::ast_node_pair) {
        lavi::lang::hash map(this);

        const lavi::lang::parser::ast_node* name_node = node.child_from_type(lavi::lang::parser::ast_node_type::ast_node_declname);
        const lavi::lang::parser::ast_node* value_node = node.child_from_type(lavi::lang::parser::ast_node_type::ast_node_valuedecl);

        std::shared_ptr<lavi::lang::object> key   = node_to_object(name_node->childrens().front());
        std::shared_ptr<lavi::lang::object> value = node_to_object(value_node->childrens().front());

        map.set(key, value);

        return lavi::lang::api::to_object(this, map);
    } else if(node.type() == lavi::lang::parser::ast_node_type::ast_node_hashdecl) {
        return execute(node);
    } else if(node.type() == lavi::lang::parser::ast_node_type::ast_node_interpolated_string) {
        return execute(node);
    }

    lavi::lang::error::internal("Unknown node type ({})", (int)node.type());

    return nullptr;
}

void lavi::lang::interpreter::load_extension(lavi::lang::extension* extension)
{
    extension->load(this);
    extensions.push_back(extension);
}

void lavi::lang::interpreter::push_context()
{
    lavi::console::log_debug("Pushing context");

    stack.push_back(std::make_shared<interpreter_context>());
    update_current_context();
}

void lavi::lang::interpreter::push_block_context()
{
    lavi::console::log_debug("Pushing block context");

    auto ctx = std::make_shared<interpreter_context>();
    ctx->is_block_context = true;
    ctx->given_block = current_context->given_block;
    ctx->given_block_lexical_context = current_context->given_block_lexical_context;

    // Always inherit the immediate parent context as lexical_parent so that a loop or
    // other block inside a function can see the function's own variables.
    // Walking past a function boundary is prevented naturally because push_context()
    // never sets lexical_parent on function contexts (their lexical_parent stays null).
    if(stack.size() >= 2) {
        ctx->lexical_parent = stack.back();
    }

    stack.push_back(ctx);
    update_current_context();
}

void lavi::lang::interpreter::push_context(std::shared_ptr<lavi::lang::object> object)
{
    lavi::console::log_debug("Pushing context with object");

    if(object) {
        push_context_with_object(object);
    } else {
        push_context();
    }

    update_current_context();
}

void lavi::lang::interpreter::push_context_with_object(std::shared_ptr<lavi::lang::object> object)
{
    std::string_view class_name = object->klass ? object->klass->name : "null";
    auto new_context = std::make_shared<interpreter_context>();

    stack.push_back(new_context);

    update_current_context();

    if(object->klass == lavi::lang::class_class) {
        lavi::console::log_debug("Pushing context with {}", class_name);

        auto klass = object->as<std::shared_ptr<lavi::lang::klass>>();
        new_context->klass = klass;
        class_name = new_context->klass->name;

        for(auto& variable : klass->variables) {
            new_context->variables[variable.first] = variable.second;
        }

        for(auto& function : klass->functions) {
            new_context->functions[function.first] = function.second;
        }

        for(auto& function : klass->inline_functions) {
            new_context->inline_functions[function.first] = function.second;
        }

    } else {
        lavi::console::log_debug("Pushing context with {}#{}", class_name, (void*)object.get());

        set_current_context_object(object);
    }
}

void lavi::lang::interpreter::set_current_context_object(std::shared_ptr<lavi::lang::object> object)
{
    auto base = object->base_instance;

    if(base) {
        set_current_context_object(base);
    }

    // Keeps the object alive while the context is alive
    current_context->self = object;

    for(auto& variable : object->variables) {
        current_context->variables[variable.first] = variable.second;
    }

    for(auto& function : object->functions) {
        current_context->functions[function.first] = function.second;
    }

    for(auto& function : object->inline_functions) {
        current_context->inline_functions[function.first] = function.second;
    }
}

void lavi::lang::interpreter::pop_context()
{
    if(stack.size() == 1) {
        throw std::runtime_error("interpreter: unexpected end of context stack");
    }

    lavi::console::log_debug("Popping context");

    stack.pop_back();
    update_current_context();
}

void lavi::lang::interpreter::update_current_context()
{
    if(stack.empty()) {
        current_context = nullptr;
        global_context = nullptr;
        return;
    }

    global_context = stack.front();
    previous_context = current_context;
    current_context = stack.back();

    auto respond_to = current_context->functions.find("respond_to?");

    if(respond_to == current_context->functions.end()) {
        current_context->functions["respond_to?"] = std::make_shared<lavi::lang::function>("respond_to?", std::initializer_list<std::string>{ "function" }, [](lavi::lang::interpreter* interpreter) {
            auto function_name_object = interpreter->current_context->positional_params[0];
            auto& function_name = function_name_object->as<std::string>();

            auto variable_it = interpreter->current_context->variables.find(function_name);

            if(variable_it != interpreter->current_context->variables.end()) {
                return lavi::lang::api::to_object(interpreter, true);
            }

            auto function_it = interpreter->current_context->functions.find(function_name);

            if(function_it != interpreter->current_context->functions.end()) {
                return lavi::lang::api::to_object(interpreter, true);
            }

            auto global_variable_it = interpreter->global_context->variables.find(function_name);

            if(global_variable_it != interpreter->global_context->variables.end()) {
                return lavi::lang::api::to_object(interpreter, true);
            }

            auto global_function_it = interpreter->global_context->functions.find(function_name);

            if(global_function_it != interpreter->global_context->functions.end()) {
                return lavi::lang::api::to_object(interpreter, true);
            }

            return lavi::lang::api::to_object(interpreter, false);
        });
    }

    auto send_function_it = current_context->functions.find("send");

    if(send_function_it == current_context->functions.end()) {
        current_context->functions["send"] = std::make_shared<lavi::lang::function>("send", std::initializer_list<std::string>{ "function"}, [](lavi::lang::interpreter* interpreter) -> std::shared_ptr<lavi::lang::object> {
            auto function_object = interpreter->current_context->positional_params[0];
            auto& function_string = function_object->as<std::string>();

            std::shared_ptr<lavi::lang::object> self = interpreter->current_context->self;
            return lavi::lang::api::call(interpreter, function_string, self);
        });
    }
}