#include <andy/lang/lang.hpp>
#include <andy/lang/interpreter.hpp>

#include "add_operators.hpp"

std::shared_ptr<lavi::lang::structure> create_double_class(lavi::lang::interpreter* interpreter)
{
    std::shared_ptr<lavi::lang::structure> DoubleClass = std::make_shared<lavi::lang::structure>("Double");

        DoubleClass->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
            double i = interpreter->current_context->self->as<double>();
            
            if(i == 0) {
                return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
        });

        DoubleClass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
            double value = interpreter->current_context->self->as<double>();

            return lavi::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(std::to_string(value)));
        });


    lavi::lang::add_operators<double>(DoubleClass, interpreter);

    return DoubleClass;
}