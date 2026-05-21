#include <andy/lang/lang.hpp>
#include <andy/lang/interpreter.hpp>

#include "add_operators.hpp"

std::shared_ptr<lavi::lang::structure> create_function_class(lavi::lang::interpreter* interpreter)
{
    std::shared_ptr<lavi::lang::structure> functionClass = std::make_shared<lavi::lang::structure>("Function");

    // No functions to add initially

    lavi::lang::add_operators<float>(functionClass, interpreter);

    return functionClass;
}