#include <andy/lang/lang.hpp>
#include <andy/lang/interpreter.hpp>
#include <andy/lang/api.hpp>
#include <andy/lang/error.hpp>

std::shared_ptr<lavi::lang::structure> create_true_class(lavi::lang::interpreter* interpreter)
{
    auto TrueClass = std::make_shared<lavi::lang::structure>("True");

        TrueClass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
                return lavi::lang::api::to_object(interpreter, "true");
    });

        TrueClass->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [TrueClass](lavi::lang::interpreter* interpreter) {
                return std::make_shared<lavi::lang::object>(TrueClass);
    });
    
        TrueClass->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
                return interpreter->current_context->positional_params[0]->cls == interpreter->TrueClass ? std::make_shared<lavi::lang::object>(interpreter->TrueClass) : std::make_shared<lavi::lang::object>(interpreter->FalseClass);
    });
    
        TrueClass->instance_functions["||"] = std::make_shared<lavi::lang::function>("||", std::initializer_list<std::string>{"other"}, [TrueClass](lavi::lang::interpreter* interpreter) {
                return std::make_shared<lavi::lang::object>(TrueClass);
    });
    
        TrueClass->instance_functions["!"] = std::make_shared<lavi::lang::function>("!", [](lavi::lang::interpreter* interpreter) {
                return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
    });
    
    TrueClass->instance_functions["&&"] = std::make_shared<lavi::lang::function>("&&", std::initializer_list<std::string>{"other"}, [TrueClass](lavi::lang::interpreter* interpreter) {
        std::shared_ptr<lavi::lang::object> other = interpreter->current_context->positional_params[0];

        return lavi::lang::api::to_object(interpreter, lavi::lang::api::is_truthy(interpreter, other));
    });
    
    return TrueClass;
}