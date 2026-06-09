#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>

std::shared_ptr<lavi::lang::klass> create_null_class()
{
    lavi::lang::null_class = lavi::lang::klass::create_builtin("Null");

    lavi::lang::null_class->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
        return std::make_shared<lavi::lang::object>( lavi::lang::false_class );
    });
    
    lavi::lang::null_class->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
        std::string str = "null";
        return lavi::lang::object::instantiate( interpreter, lavi::lang::string_class, std::move(str) );
    });
    
    lavi::lang::null_class->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
        if(interpreter->current_context->positional_params[0]->cls == lavi::lang::null_class) {
            return std::make_shared<lavi::lang::object>( lavi::lang::true_class );
        } else {
            return std::make_shared<lavi::lang::object>( lavi::lang::false_class );
        }
    });
    
    return lavi::lang::null_class;
}