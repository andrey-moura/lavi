#include "lavi/lang/lang.hpp"
#include "lavi/lang/interpreter.hpp"

#include <andy/console.hpp>

extern std::shared_ptr<lavi::lang::structure> create_false_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_andy_config_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_array_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_class_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_hash_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_directory_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_double_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_false_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_file_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_float_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_integer_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_null_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_path_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_string_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_system_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_true_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_function_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_exception_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_no_function_error_class(lavi::lang::interpreter*);
extern std::shared_ptr<lavi::lang::structure> create_runtime_error_class(lavi::lang::interpreter*);

void create_std_functions(lavi::lang::interpreter*);

void lavi::lang::structure::create_structures(lavi::lang::interpreter* interpreter)
{
    // FIRST
    interpreter->load(interpreter->ClassClass       = create_class_class       (interpreter) );
    interpreter->load(interpreter->FalseClass       = create_false_class       (interpreter) );
    interpreter->load(interpreter->TrueClass        = create_true_class        (interpreter) );
    interpreter->load(interpreter->StringClass      = create_string_class      (interpreter) );
    interpreter->load(interpreter->IntegerClass     = create_integer_class     (interpreter) );
    interpreter->load(interpreter->DoubleClass      = create_double_class      (interpreter) );
    interpreter->load(interpreter->FloatClass       = create_float_class       (interpreter) );
    interpreter->load(interpreter->FileClass        = create_file_class        (interpreter) );
    interpreter->load(interpreter->ArrayClass       = create_array_class       (interpreter) );
    interpreter->load(interpreter->NullClass        = create_null_class        (interpreter) );
    interpreter->load(interpreter->HashClass        = create_hash_class        (interpreter) );
    interpreter->load(interpreter->SystemClass      = create_system_class      (interpreter) );
    interpreter->load(interpreter->PathClass        = create_path_class        (interpreter) );
    interpreter->load(interpreter->AndyConfigClass  = create_andy_config_class (interpreter) );
    interpreter->load(interpreter->FunctionClass    = create_function_class    (interpreter) );
    interpreter->load(interpreter->ExceptionClass   = create_exception_class   (interpreter) );
    interpreter->load(interpreter->NoFunctionErrorClass = create_no_function_error_class(interpreter) );
    interpreter->load(interpreter->RuntimeErrorClass = create_runtime_error_class(interpreter) );

    // These are not named on Interpreter because they are not used too often
    // Some of the one which are named should be moved to here soon.
    interpreter->load(create_directory_class(interpreter));
    create_std_functions(interpreter);
}

lavi::lang::structure::structure(std::string_view __name, std::vector<lavi::lang::function> __methods)
    : name(std::move(__name))
{
    for(auto& method : __methods) {
        if(method.storage_type == function_storage_type::class_function) {
            functions[method.name] = std::make_shared<lavi::lang::function>(std::move(method));
        } else {
            instance_functions[method.name] = std::make_shared<lavi::lang::function>(std::move(method));
        }
    }

    lavi::console::log_debug("{}#Class created", name);
}

lavi::lang::structure::~structure()
{
    lavi::console::log_debug("{}#Class destroyed", name);
}