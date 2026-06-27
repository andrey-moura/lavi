#include <lavi/lang/lang.hpp>
#include <lavi/lang/classes.hpp>
#include <lavi/lang/api.hpp>

#include "add_operators.hpp"

void create_integer_class()
{
    lavi::lang::integer_class = lavi::lang::klass::create_builtin("Integer");

    lavi::lang::integer_class->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
        int i = interpreter->current_context->self->as<int>();
        
        if(i == 0) {
            return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
        }

        return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
    });

    lavi::lang::integer_class->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
        int value = interpreter->current_context->self->as<int>();

        return lavi::lang::api::to_object(interpreter, std::to_string(value));
    });

    lavi::lang::integer_class->instance_functions["inspect"] = std::make_shared<lavi::lang::function>("inspect", [](lavi::lang::interpreter* interpreter) {
        int value = interpreter->current_context->self->as<int>();

        return lavi::lang::api::to_object(interpreter, std::to_string(value));
    });

    lavi::lang::integer_class->instance_functions["positive?"] = std::make_shared<lavi::lang::function>("positive?", [](lavi::lang::interpreter* interpreter) {
        int value = interpreter->current_context->self->as<int>();

        if(value == 0) {
            return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
        }

        return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
    });

    lavi::lang::add_operators<int>(lavi::lang::integer_class);
}