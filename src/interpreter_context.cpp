#include "lavi/lang/interpreter_context.hpp"
#include "lavi/lang/class.hpp"

std::string lavi::lang::interpreter_context::fully_qualified_name(std::string_view name) const {
    if(klass) {
        return klass->name + "::" + std::string(name);
    }
    return std::string(name);
}