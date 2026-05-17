#include <andy/lang/object.hpp>

#include <andy/lang/class.hpp>
#include <andy/lang/function.hpp>
#include <andy/lang/interpreter.hpp>

#include <andy/console.hpp>

andy::lang::object::object(std::shared_ptr<andy::lang::structure> c)
{
    cls = c;
    andy::console::log_debug(default_string_representation() + " created");
}

andy::lang::object::~object()
{
    if(native_destructor) {
        native_destructor(this);
    }

    andy::console::log_debug(default_string_representation() + " destroyed");
}

const std::string& andy::lang::object::default_string_representation()
{
    if(string_representation_cache.empty()) {
        if(cls) {
            string_representation_cache += cls->name;
            string_representation_cache.push_back('#');
        }
        string_representation_cache += std::to_string((uintptr_t)this);
    }

    return string_representation_cache;
}

void andy::lang::object::initialize(andy::lang::interpreter* interpreter)
{
    self = shared_from_this().get();
    for(auto& instance_variable : cls->instance_variables) {
        variables[instance_variable.first] = andy::lang::object::create(interpreter, interpreter->NullClass);
        if(instance_variable.second) {
            interpreter->execute(*instance_variable.second);
        }
    }
    functions = cls->instance_functions;
    inline_functions = cls->instance_inline_functions;
}

void andy::lang::object::initialize(andy::lang::interpreter *interpreter, andy::lang::function_call new_call)
{
    initialize(interpreter);
}

void andy::lang::object::log_native_destructor()
{
    andy::console::log_debug("{}#{} native destructor", cls->name, (void*)this);
}