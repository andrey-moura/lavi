#include "lavi/lang/lang.hpp"
#include "lavi/lang/interpreter.hpp"
#include "lavi/lang/classes.hpp"
#include "lavi/lang/api.hpp"

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
extern void create_std_class();

// Define global classes
namespace lavi
{
  namespace lang
  {
    std::vector<std::shared_ptr<lavi::lang::klass>> builtin_classes;
    bool classes_created = false;

    std::shared_ptr<lavi::lang::klass> std_class;
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

void lavi::lang::klass::create_builtin_classes()
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
    create_std_class();
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
    auto klass = std::make_shared<lavi::lang::klass>(name);

    klass->functions["subclasses"] = std::make_shared<lavi::lang::function>("subclasses", [klass](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>> subclasses;
        subclasses.reserve(klass->deriveds.size());

        for(auto& klass : klass->deriveds) {
            subclasses.push_back(lavi::lang::api::to_object(interpreter, klass));
        }

        return lavi::lang::api::to_object(interpreter, std::move(subclasses));
    });

    auto type_instance_function = klass->instance_functions.find("class");

    if(type_instance_function == klass->instance_functions.end()) {
        klass->instance_functions["class"] = std::make_shared<lavi::lang::function>("class", [klass](lavi::lang::interpreter* interpreter) {
            return lavi::lang::api::to_object(interpreter, klass);
        });
    }

    auto is_a_instance_function = klass->instance_functions.find("is_a?");

    if(is_a_instance_function == klass->instance_functions.end()) {
        klass->instance_functions["is_a?"] = std::make_shared<lavi::lang::function>("is_a?", std::initializer_list<std::string>{ "other" }, [klass](lavi::lang::interpreter* interpreter) {
            auto other_object = interpreter->current_context->positional_params[0];
            auto other_class = other_object->as<std::shared_ptr<lavi::lang::klass>>();

            return lavi::lang::api::to_object(interpreter, lavi::lang::api::is_a(interpreter, interpreter->current_context->self, klass));
        });
    }

    auto init_instance_function = klass->instance_functions.find("init");

    if(init_instance_function == klass->instance_functions.end()) {
        klass->instance_functions["init"] = std::make_shared<lavi::lang::function>("init", [](lavi::lang::interpreter* interpreter) {
            auto base_instance = interpreter->current_context->self->base_instance;
            if(base_instance) {
                lavi::lang::api::call(
                    interpreter,
                    "init",
                    base_instance,
                    interpreter->current_context->positional_params,
                    interpreter->current_context->named_params
                );
            }
            return nullptr;
        });
    }

    auto to_string_instance_function = klass->instance_functions.find("to_string");

    if(to_string_instance_function == klass->instance_functions.end()) {
        klass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [klass](lavi::lang::interpreter* interpreter) {
            return lavi::lang::api::to_object(interpreter, interpreter->current_context->self->default_string_representation());
        });
    }

    auto hash_instance_function = klass->instance_functions.find("hash");

    if(hash_instance_function == klass->instance_functions.end()) {
        klass->instance_functions["hash"] = std::make_shared<lavi::lang::function>("hash", [klass](lavi::lang::interpreter* interpreter) {
            // Default hash: the pointer of the object
            auto object = interpreter->current_context->self;
            void* ptr = object.get();
            std::hash<void*> hasher;
            size_t hash_value = hasher(ptr);
            return lavi::lang::api::to_object(interpreter, (int)hash_value);
        });
    }

    auto eq_instance_function = klass->instance_functions.find("==");

    if(eq_instance_function == klass->instance_functions.end()) {
        klass->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", [klass](lavi::lang::interpreter* interpreter) {
            auto object = interpreter->current_context->self.get();
            auto other_object = interpreter->current_context->positional_params[0].get();

            // Default equality: compare the pointers of the objects
            return lavi::lang::api::to_object(interpreter, object == other_object);
        });
    }

    auto inspect_instance_function = klass->instance_functions.find("inspect");

    if(inspect_instance_function == klass->instance_functions.end()) {
        klass->instance_functions["inspect"] = std::make_shared<lavi::lang::function>("inspect", [klass](lavi::lang::interpreter* interpreter) {
            auto object = interpreter->current_context->self;
            return lavi::lang::api::call(interpreter, "to_string", object);
        });
    }

    auto class_instance_function = klass->instance_functions.find("class");

    if(class_instance_function == klass->instance_functions.end()) {
        klass->instance_functions["class"] = std::make_shared<lavi::lang::function>("class", [](lavi::lang::interpreter* interpreter) {
            return lavi::lang::api::to_object(interpreter, interpreter->current_context->self->klass);
        });
    }

    auto to_string_function = klass->functions.find("to_string");

    if(to_string_function == klass->functions.end()) {
        klass->functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [klass](lavi::lang::interpreter* interpreter) {
            return lavi::lang::api::to_object(interpreter, klass->name);
        });
    }

    auto name_function = klass->functions.find("name");

    if(name_function == klass->functions.end()) {
        klass->functions["name"] = std::make_shared<lavi::lang::function>("name", [klass](lavi::lang::interpreter* interpreter) {
            return lavi::lang::api::to_object(interpreter, klass->name);
        });
    }

    auto eq_function = klass->functions.find("==");

    if(eq_function == klass->functions.end()) {
        klass->functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{ "other" }, [klass](lavi::lang::interpreter* interpreter) {
            if(interpreter->current_context->positional_params[0]->klass != lavi::lang::class_class) {
                return lavi::lang::api::to_object(interpreter, false);
            }

            return lavi::lang::api::to_object(
                interpreter,
                interpreter->current_context->positional_params[0]->as<std::shared_ptr<lavi::lang::klass>>() == interpreter->current_context->klass
            );
        });
    }

    return klass;
}

std::shared_ptr<lavi::lang::klass> lavi::lang::klass::create_builtin(std::string_view name)
{
    auto klass = create(name);
    builtin_classes.push_back(klass);
    return klass;
}