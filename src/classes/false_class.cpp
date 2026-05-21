#include <andy/lang/lang.hpp>
#include <andy/lang/interpreter.hpp>
#include <andy/lang/error.hpp>
#include <andy/lang/api.hpp>

std::shared_ptr<lavi::lang::structure> create_false_class(lavi::lang::interpreter* interpreter)
{
    auto FalseClass = std::make_shared<lavi::lang::structure>("False");

        FalseClass->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [FalseClass](lavi::lang::interpreter* interpreter) {
                return std::make_shared<lavi::lang::object>(FalseClass);
    });

        FalseClass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
                return lavi::lang::api::to_object(interpreter, "false");
    });

    FalseClass->instance_functions["||"] = std::make_shared<lavi::lang::function>("||", std::initializer_list<std::string>{"other"}, [FalseClass](lavi::lang::interpreter* interpreter) {
        auto other = interpreter->current_context->positional_params[0];
        //return other->cls->instance_functions["present?"]->call(other);
        lavi::lang::error::internal("Temporary disabled code reached at " + std::string(__FILE__) + ":" + std::to_string(__LINE__));

        return std::make_shared<lavi::lang::object>(FalseClass);
    });
    
        FalseClass->instance_functions["!"] = std::make_shared<lavi::lang::function>("!", [](lavi::lang::interpreter* interpreter) {
                return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
    });
    
        FalseClass->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{"other"}, [FalseClass](lavi::lang::interpreter* interpreter) {
                auto other = interpreter->current_context->positional_params[0];
        return std::make_shared<lavi::lang::object>(other->cls == FalseClass ? interpreter->TrueClass : interpreter->FalseClass);
    });

    FalseClass->instance_functions["&&"] = std::make_shared<lavi::lang::function>("&&", std::initializer_list<std::string>{"other"}, [FalseClass](lavi::lang::interpreter* interpreter) {
        return lavi::lang::api::to_object(interpreter, false);
    });
    
    return FalseClass;
}