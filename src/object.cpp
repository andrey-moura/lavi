#include <lavi/lang/object.hpp>

#include <lavi/lang/class.hpp>
#include <lavi/lang/function.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/api.hpp>

#include <andy/console.hpp>

lavi::lang::object::object(std::shared_ptr<lavi::lang::klass> c)
{
    klass = c;
    lavi::console::log_debug(default_string_representation() + " created");
}

lavi::lang::object::~object()
{
    if(native_destructor) {
        native_destructor(this);
    }

    lavi::console::log_debug(default_string_representation() + " destroyed");
}

const std::string& lavi::lang::object::default_string_representation()
{
    string_representation_cache.clear();

    if(klass) {
        string_representation_cache += klass->name;
        string_representation_cache.push_back('#');
    }

    string_representation_cache += std::to_string((uintptr_t)this);

    return string_representation_cache;
}

void lavi::lang::object::initialize(lavi::lang::interpreter* interpreter)
{
    for(auto& instance_variable : klass->instance_variables) {
        if(instance_variable.second.is_undefined()) {
            variables[instance_variable.first] = lavi::lang::api::to_object(interpreter, nullptr);
        } else {
            variables[instance_variable.first] = interpreter->execute(instance_variable.second);
        }
    }
    functions = klass->instance_functions;
    inline_functions = klass->instance_inline_functions;
}

void lavi::lang::object::log_native_destructor()
{
    lavi::console::log_debug("{}#{} native destructor", klass->name, (void*)this);
}