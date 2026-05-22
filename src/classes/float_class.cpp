#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>

#include "add_operators.hpp"

std::shared_ptr<lavi::lang::structure> create_float_class(lavi::lang::interpreter* interpreter)
{
    std::shared_ptr<lavi::lang::structure> FloatClass = std::make_shared<lavi::lang::structure>("Float");

        FloatClass->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
            float i = interpreter->current_context->self->as<float>();
            
            if(i == 0) {
                return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
        });

        FloatClass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
            float value = interpreter->current_context->self->as<float>();

            return lavi::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(std::to_string(value)));
        });


    lavi::lang::add_operators<float>(FloatClass, interpreter);

    return FloatClass;
}