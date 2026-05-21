#include <andy/lang/lang.hpp>
#include <andy/lang/interpreter.hpp>
#include <andy/lang/api.hpp>

#include "add_operators.hpp"

std::shared_ptr<lavi::lang::structure> create_integer_class(lavi::lang::interpreter* interpreter)
{
    std::shared_ptr<lavi::lang::structure> IntegerClass = std::make_shared<lavi::lang::structure>("Integer");

    IntegerClass->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
        int i = interpreter->current_context->self->as<int>();
        
        if(i == 0) {
            return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
        }

        return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
    });

    IntegerClass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
        int value = interpreter->current_context->self->as<int>();

        return lavi::lang::api::to_object(interpreter, std::to_string(value));
    });

    IntegerClass->instance_functions["inspect"] = std::make_shared<lavi::lang::function>("inspect", [](lavi::lang::interpreter* interpreter) {
        int value = interpreter->current_context->self->as<int>();

        return lavi::lang::api::to_object(interpreter, std::to_string(value));
    });

    IntegerClass->instance_functions["positive?"] = std::make_shared<lavi::lang::function>("positive?", [](lavi::lang::interpreter* interpreter) {
        int value = interpreter->current_context->self->as<int>();

        if(value == 0) {
            return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
        }

        return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
    });

    lavi::lang::add_operators<int>(IntegerClass, interpreter);

    return IntegerClass;
}