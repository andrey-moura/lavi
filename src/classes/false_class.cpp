#include <lavi/lang/lang.hpp>
#include <lavi/lang/classes.hpp>
#include <lavi/lang/error.hpp>
#include <lavi/lang/api.hpp>

void create_false_class()
{
    lavi::lang::false_class = lavi::lang::klass::create_builtin("False");

        lavi::lang::false_class->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
                return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
    });

        lavi::lang::false_class->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
                return lavi::lang::api::to_object(interpreter, "false");
    });

    lavi::lang::false_class->instance_functions["||"] = std::make_shared<lavi::lang::function>("||", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
        auto other = interpreter->current_context->positional_params[0];
        return lavi::lang::api::to_object(interpreter, lavi::lang::api::is_truthy(interpreter, other));
    });
    
        lavi::lang::false_class->instance_functions["!"] = std::make_shared<lavi::lang::function>("!", [](lavi::lang::interpreter* interpreter) {
                return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
    });
    
        lavi::lang::false_class->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
                auto other = interpreter->current_context->positional_params[0];
        return std::make_shared<lavi::lang::object>(other->cls == lavi::lang::false_class ? lavi::lang::true_class : lavi::lang::false_class);
    });

    lavi::lang::false_class->instance_functions["&&"] = std::make_shared<lavi::lang::function>("&&", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
        return lavi::lang::api::to_object(interpreter, false);
    });
}