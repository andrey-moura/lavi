#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>

std::shared_ptr<lavi::lang::structure> create_null_class(lavi::lang::interpreter* interpreter)
{
    auto NullClass = std::make_shared<lavi::lang::structure>("Null");

    NullClass->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [NullClass](lavi::lang::interpreter* interpreter) {
        return std::make_shared<lavi::lang::object>( interpreter->FalseClass );
    });
    
    NullClass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
        std::string str = "null";
        return lavi::lang::object::instantiate( interpreter, interpreter->StringClass, std::move(str) );
    });
    
    NullClass->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
        if(interpreter->current_context->positional_params[0]->cls == interpreter->NullClass) {
            return std::make_shared<lavi::lang::object>( interpreter->TrueClass );
        } else {
            return std::make_shared<lavi::lang::object>( interpreter->FalseClass );
        }
    });
    
    return NullClass;
}