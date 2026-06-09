#include <lavi/lang/lang.hpp>
#include <lavi/lang/classes.hpp>
#include <lavi/lang/api.hpp>
#include <lavi/lang/error.hpp>

void create_true_class()
{
    lavi::lang::true_class = lavi::lang::klass::create_builtin("True");

        lavi::lang::true_class->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
                return lavi::lang::api::to_object(interpreter, "true");
    });

        lavi::lang::true_class->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
                return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
    });
    
        lavi::lang::true_class->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
                return interpreter->current_context->positional_params[0]->cls == lavi::lang::true_class ? std::make_shared<lavi::lang::object>(lavi::lang::true_class) : std::make_shared<lavi::lang::object>(lavi::lang::false_class);
    });
    
        lavi::lang::true_class->instance_functions["||"] = std::make_shared<lavi::lang::function>("||", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
                return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
    });
    
        lavi::lang::true_class->instance_functions["!"] = std::make_shared<lavi::lang::function>("!", [](lavi::lang::interpreter* interpreter) {
                return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
    });
    
    lavi::lang::true_class->instance_functions["&&"] = std::make_shared<lavi::lang::function>("&&", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
        std::shared_ptr<lavi::lang::object> other = interpreter->current_context->positional_params[0];

        return lavi::lang::api::to_object(interpreter, lavi::lang::api::is_truthy(interpreter, other));
    });
}