#include <lavi/lang/lang.hpp>
#include <lavi/lang/classes.hpp>

#include "add_operators.hpp"

void create_double_class()
{
    lavi::lang::double_class = lavi::lang::klass::create_builtin("Double");

        lavi::lang::double_class->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
            double i = interpreter->current_context->self->as<double>();
            
            if(i == 0) {
                return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
            }

            return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
        });

        lavi::lang::double_class->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
            double value = interpreter->current_context->self->as<double>();

            return lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, std::move(std::to_string(value)));
        });


    lavi::lang::add_operators<double>(lavi::lang::double_class);
}