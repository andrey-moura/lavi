#include <lavi/lang/lang.hpp>
#include <lavi/lang/classes.hpp>

#include "add_operators.hpp"

void create_float_class()
{
    lavi::lang::float_class = lavi::lang::klass::create_builtin("Float");

        lavi::lang::float_class->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
            float i = interpreter->current_context->self->as<float>();
            
            if(i == 0) {
                return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
            }

            return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
        });

        lavi::lang::float_class->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
            float value = interpreter->current_context->self->as<float>();

            return lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, std::move(std::to_string(value)));
        });


    lavi::lang::add_operators<float>(lavi::lang::float_class);
}