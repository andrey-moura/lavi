#pragma once
#include <lavi/lang/class.hpp>
#include <lavi/lang/function.hpp>
#include <lavi/lang/object.hpp>
#include "lavi/lang/api.hpp"


#define UNARY_OPERATOR_INLINE(op, T) \
    [](lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object>& object, const lavi::lang::parser::ast_node& source_code) { \
        T& value = object->as<T>(); \
        value op; \
        return object; \
    }

#define BINARY_ASSIGNMENT_OPERATOR_INLINE(op, T) \
    [](lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object>& object, const lavi::lang::parser::ast_node& source_code) { \
        const auto* params_node = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_params); \
        if (!params_node || params_node->childrens().size() != 1) { \
            throw std::runtime_error(std::string(object->klass->name) + "::operator " #op " requires exactly one parameter"); \
        } \
        const auto& param = params_node->childrens()[0]; \
        T& value = object->as<T>(); \
        if (param.type() != lavi::lang::parser::ast_node_type::ast_node_valuedecl || param.token().type != lavi::lang::lexer::token_type::token_literal) { \
            throw std::runtime_error(std::string(object->klass->name) + "::operator " #op " requires a numeric literal"); \
        } \
        switch(param.token().kind) { \
        case lavi::lang::lexer::token_kind::token_integer: \
            value op param.token().integer_literal; \
            break; \
        case lavi::lang::lexer::token_kind::token_float: \
            value op param.token().float_literal; \
            break; \
        case lavi::lang::lexer::token_kind::token_double: \
            value op param.token().double_literal; \
            break; \
        default: \
            throw std::runtime_error(std::string(object->klass->name) + "::operator " #op " requires a numeric literal"); \
        } \
        return object; \
    }
#define BINARY_COMPARISON_OPERATOR_INLINE(op, T) \
    [](lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object>& object, const lavi::lang::parser::ast_node& source_code) { \
        const auto* params_node = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_params); \
        if (!params_node || params_node->childrens().size() != 1) { \
            throw std::runtime_error(std::string(object->klass->name) + "::operator " #op " requires exactly one parameter"); \
        } \
        const auto& param = params_node->childrens()[0]; \
        T& value = object->as<T>(); \
        bool comparison_result = false; \
        if (param.type() != lavi::lang::parser::ast_node_type::ast_node_valuedecl || param.token().type != lavi::lang::lexer::token_type::token_literal) { \
            throw std::runtime_error(std::string(object->klass->name) + "::operator " #op " requires a numeric literal"); \
        } \
        switch(param.token().kind) { \
        case lavi::lang::lexer::token_kind::token_integer: \
            comparison_result = value op param.token().integer_literal; \
            break; \
        case lavi::lang::lexer::token_kind::token_float: \
            comparison_result = value op param.token().float_literal; \
            break; \
        case lavi::lang::lexer::token_kind::token_double: \
            comparison_result = value op param.token().double_literal; \
            break; \
        default: \
            throw std::runtime_error(std::string(object->klass->name) + "::operator " #op " requires a numeric literal"); \
        } \
        return lavi::lang::api::to_object(interpreter, comparison_result); \
    }
#define BINARY_ARITHMETIC_OPERATOR_INLINE(op, T) \
    [](lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object>& object, const lavi::lang::parser::ast_node& source_code) { \
        const auto* params_node = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_params); \
        if (!params_node || params_node->childrens().size() != 1) { \
            throw std::runtime_error(std::string(object->klass->name) + "::operator " #op " requires exactly one parameter"); \
        } \
        const auto& param = params_node->childrens()[0]; \
        T value = object->as<T>(); \
        if (param.type() != lavi::lang::parser::ast_node_type::ast_node_valuedecl || param.token().type != lavi::lang::lexer::token_type::token_literal) { \
            throw std::runtime_error(std::string(object->klass->name) + "::operator " #op " requires a numeric literal"); \
        } \
        switch(param.token().kind) { \
        case lavi::lang::lexer::token_kind::token_integer: \
            value = value op param.token().integer_literal; \
            break; \
        case lavi::lang::lexer::token_kind::token_float: \
            value = value op param.token().float_literal; \
            break; \
        case lavi::lang::lexer::token_kind::token_double: \
            value = value op param.token().double_literal; \
            break; \
        default: \
            throw std::runtime_error(std::string(object->klass->name) + "::operator " #op " requires a numeric literal"); \
        } \
        return lavi::lang::object::instantiate(interpreter, object->klass, value); \
    }

#define UNARY_OPERATOR(op, T) \
    lavi::lang::function(#op, [](lavi::lang::interpreter* interpreter) { \
        T& value = interpreter->current_context->self->as<T>(); \
        value op; \
        return interpreter->current_context->self->shared_from_this(); \
    })

#define BINARY_ASSIGNMENT_OPERATOR(op, T) \
    lavi::lang::function(#op, { "other" }, [](lavi::lang::interpreter* interpreter) { \
        const auto& params = interpreter->current_context->positional_params; \
        T& value = interpreter->current_context->self->as<T>(); \
        auto& param = params[0]; \
        if (params[0]->klass == lavi::lang::integer_class) { \
            value op params[0]->as<int>(); \
        } else if (params[0]->klass == lavi::lang::float_class) { \
            value op params[0]->as<float>(); \
        } else if (params[0]->klass == lavi::lang::double_class) { \
            value op params[0]->as<double>(); \
        } else { \
            throw std::runtime_error("undefined operator " #op " (" + std::string(interpreter->current_context->self->klass->name) + ", " + std::string(params[0]->klass->name) + ")"); \
        } \
        return interpreter->current_context->self->shared_from_this(); \
    })

#define BINARY_COMPARISON_OPERATOR(op, T) \
    lavi::lang::function(#op, { "other" }, [](lavi::lang::interpreter* interpreter) { \
        const auto& params = interpreter->current_context->positional_params; \
        std::shared_ptr<lavi::lang::object> other = params[0]; \
        T& value = interpreter->current_context->self->as<T>(); \
        bool comparison_result = false; \
        if (params[0]->klass == lavi::lang::integer_class) { \
            comparison_result = value op params[0]->as<int>(); \
        } else if (params[0]->klass == lavi::lang::float_class) { \
            comparison_result = value op params[0]->as<float>(); \
        } else if (params[0]->klass == lavi::lang::double_class) { \
            comparison_result = value op params[0]->as<double>(); \
        } else { \
            throw std::runtime_error("undefined operator " #op " (" + std::string(interpreter->current_context->self->klass->name) + ", " + std::string(params[0]->klass->name) + ")"); \
        } \
        return lavi::lang::api::to_object(interpreter, comparison_result); \
    })

#define BINARY_ARITHMETIC_OPERATOR(op, T) \
    lavi::lang::function(#op, { "other" }, [](lavi::lang::interpreter* interpreter) { \
        auto object = interpreter->current_context->self->shared_from_this(); \
        auto other = interpreter->current_context->positional_params[0]; \
        const auto& params = interpreter->current_context->positional_params; \
        T value = interpreter->current_context->self->as<T>(); \
        if (other->klass == lavi::lang::integer_class) { \
            value = value op other->as<int>(); \
        } else if (other->klass == lavi::lang::float_class) { \
            value = value op other->as<float>(); \
        } else if (other->klass == lavi::lang::double_class) { \
            value = value op other->as<double>(); \
        } else { \
            throw std::runtime_error("undefined operator " #op " (" + std::string(interpreter->current_context->self->klass->name) + ", " + std::string(params[0]->klass->name) + ")"); \
        } \
        return lavi::lang::object::instantiate(interpreter, object->klass, value); \
    })

namespace lavi
{
    namespace lang  
    {
        template<typename T>
        void add_operators(std::shared_ptr<lavi::lang::klass> klass)
        {
            std::map<std::string_view, lavi::lang::inline_function> inline_functions = {
                { "++", UNARY_OPERATOR_INLINE(++, T) },
                { "--", UNARY_OPERATOR_INLINE(--, T) },
                { "+=", BINARY_ASSIGNMENT_OPERATOR_INLINE(+=, T) },
                { "-=", BINARY_ASSIGNMENT_OPERATOR_INLINE(-=, T) },
                { "*=", BINARY_ASSIGNMENT_OPERATOR_INLINE(*=, T) },
                { "/=", BINARY_ASSIGNMENT_OPERATOR_INLINE(/=, T) },
                { "==", BINARY_COMPARISON_OPERATOR_INLINE(==, T) },
                { "!=", BINARY_COMPARISON_OPERATOR_INLINE(!=, T) },
                { "<",  BINARY_COMPARISON_OPERATOR_INLINE(<, T)  },
                { ">",  BINARY_COMPARISON_OPERATOR_INLINE(>, T)  },
                { "<=", BINARY_COMPARISON_OPERATOR_INLINE(<=, T) },
                { ">=", BINARY_COMPARISON_OPERATOR_INLINE(>=, T) },
                { "+",  BINARY_ARITHMETIC_OPERATOR_INLINE(+, T)  },
                { "-",  BINARY_ARITHMETIC_OPERATOR_INLINE(-, T)  },
                { "*",  BINARY_ARITHMETIC_OPERATOR_INLINE(*, T)  },
                { "/",  BINARY_ARITHMETIC_OPERATOR_INLINE(/, T)  }
            };
            std::map<std::string_view, lavi::lang::function> instance_functions = {
                { "++", UNARY_OPERATOR(++, T) },
                { "--", UNARY_OPERATOR(--, T) },
                { "+=", BINARY_ASSIGNMENT_OPERATOR(+=, T) },
                { "-=", BINARY_ASSIGNMENT_OPERATOR(-=, T) },
                { "*=", BINARY_ASSIGNMENT_OPERATOR(*=, T) },
                { "/=", BINARY_ASSIGNMENT_OPERATOR(/=, T) },
                { "==", BINARY_COMPARISON_OPERATOR(==, T) },
                { "!=", BINARY_COMPARISON_OPERATOR(!=, T) },
                { "<",  BINARY_COMPARISON_OPERATOR(<, T)  },
                { ">",  BINARY_COMPARISON_OPERATOR(>, T)  },
                { "<=", BINARY_COMPARISON_OPERATOR(<=, T) },
                { ">=", BINARY_COMPARISON_OPERATOR(>=, T) },
                { "+",  BINARY_ARITHMETIC_OPERATOR(+, T)  },
                { "-",  BINARY_ARITHMETIC_OPERATOR(-, T)  },
                { "*",  BINARY_ARITHMETIC_OPERATOR(*, T)  },
                { "/",  BINARY_ARITHMETIC_OPERATOR(/, T)  },
            };
            if constexpr (std::is_same_v<T, int>) {
                klass->instance_functions["%"] = std::make_shared<lavi::lang::function>("%", std::initializer_list<std::string>{ "other" }, [](lavi::lang::interpreter* interpreter) {
                    auto object = interpreter->current_context->self->shared_from_this();
                    auto other = interpreter->current_context->positional_params[0];
                    if(other->klass != lavi::lang::integer_class) {
                        throw std::runtime_error("undefined operator % (" + std::string(object->klass->name) + ", " + std::string(other->klass->name) + ")");
                    }
                    int value = object->as<int>();
                    value %= other->as<int>();
                    return lavi::lang::object::create(interpreter, object->klass, value);
                });
                klass->instance_inline_functions["%"] = std::make_shared<lavi::lang::inline_function>([](lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object>& object, const lavi::lang::parser::ast_node& source_code) {
                    const auto* params_node = source_code.child_from_type(lavi::lang::parser::ast_node_type::ast_node_fn_params);
                    if (!params_node || params_node->childrens().size() != 1) {
                        throw std::runtime_error(std::string(object->klass->name) + "::operator % requires exactly one parameter");
                    }
                    const auto& other = params_node->childrens()[0];
                    if(other.type() != lavi::lang::parser::ast_node_type::ast_node_valuedecl ||
                       other.token().type != lavi::lang::lexer::token_type::token_literal ||
                       other.token().kind != lavi::lang::lexer::token_kind::token_integer) {
                        throw std::runtime_error(std::string(object->klass->name) + "::operator % requires an integer literal");
                    }
                    int value = object->as<int>();
                    value %= other.token().integer_literal;
                    return lavi::lang::object::create(interpreter, object->klass, value);
                });
            }
            for(auto& [name, func_ptr] : inline_functions) {
                klass->instance_inline_functions[std::string(name)] = std::make_shared<lavi::lang::inline_function>(func_ptr);
            }
            for(auto& [name, func] : instance_functions) {
                klass->instance_functions[std::string(name)] = std::make_shared<lavi::lang::function>(std::move(func));
            }
        }
    };
};