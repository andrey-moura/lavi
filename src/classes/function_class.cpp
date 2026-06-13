#include <lavi/lang/lang.hpp>
#include <lavi/lang/classes.hpp>

#include "add_operators.hpp"

void create_function_class()
{
    lavi::lang::function_class = lavi::lang::klass::create_builtin("Function");

    // No functions to add initially
}