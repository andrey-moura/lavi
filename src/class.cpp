#include "lavi/lang/lang.hpp"
#include "lavi/lang/interpreter.hpp"
#include "lavi/lang/classes.hpp"

#include <andy/console.hpp>

extern void create_false_class();
extern void create_andy_config_class();
extern void create_array_class();
extern void create_class_class();
extern void create_hash_class();
extern void create_directory_class();
extern void create_double_class();
extern void create_false_class();
extern void create_file_class();
extern void create_float_class();
extern void create_integer_class();
extern void create_null_class();
extern void create_path_class();
extern void create_string_class();
extern void create_system_class();
extern void create_true_class();
extern void create_function_class();
extern void create_exception_class();
extern void create_no_function_error_class();
extern void create_runtime_error_class();

void create_std_functions();

// Define global classes
namespace lavi
{
  namespace lang
  {
    std::vector<std::shared_ptr<lavi::lang::klass>> builtin_classes;
    bool classes_created = false;
    std::shared_ptr<lavi::lang::klass> false_class;
    std::shared_ptr<lavi::lang::klass> true_class;
    std::shared_ptr<lavi::lang::klass> string_class;
    std::shared_ptr<lavi::lang::klass> integer_class;
    std::shared_ptr<lavi::lang::klass> double_class;
    std::shared_ptr<lavi::lang::klass> float_class;
    std::shared_ptr<lavi::lang::klass> file_class;
    std::shared_ptr<lavi::lang::klass> array_class;
    std::shared_ptr<lavi::lang::klass> null_class;
    std::shared_ptr<lavi::lang::klass> hash_class;
    std::shared_ptr<lavi::lang::klass> system_class;
    std::shared_ptr<lavi::lang::klass> path_class;
    std::shared_ptr<lavi::lang::klass> andy_config_class;
    std::shared_ptr<lavi::lang::klass> class_class;
    std::shared_ptr<lavi::lang::klass> function_class;
    std::shared_ptr<lavi::lang::klass> exception_class;
    std::shared_ptr<lavi::lang::klass> no_function_error_class;
    std::shared_ptr<lavi::lang::klass> runtime_error_class;
  }
}

void lavi::lang::klass::create_structures()
{
    // FIRST
    create_class_class();
    create_false_class();
    create_true_class();
    create_string_class();
    create_integer_class();
    create_double_class();
    create_float_class();
    create_file_class();
    create_array_class();
    create_null_class();
    create_hash_class();
    create_system_class();
    create_path_class();
    create_andy_config_class();
    create_function_class();
    create_exception_class();
    create_no_function_error_class();
    create_runtime_error_class();
    // These are not named on Interpreter because they are not used too often
    // Some of the one which are named should be moved to here soon.
    create_directory_class();
    create_std_functions();
}

lavi::lang::klass::klass(std::string_view __name, std::vector<lavi::lang::function> __methods)
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

lavi::lang::klass::~klass()
{
    lavi::console::log_debug("{}#Class destroyed", name);
}

std::shared_ptr<lavi::lang::klass> lavi::lang::klass::create(std::string_view name)
{
    auto structure = std::make_shared<lavi::lang::klass>(name);
    return structure;
}

std::shared_ptr<lavi::lang::klass> lavi::lang::klass::create_builtin(std::string_view name)
{
    auto structure = create(name);
    builtin_classes.push_back(structure);
    return structure;
}