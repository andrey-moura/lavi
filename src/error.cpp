#include "lavi/lang/error.hpp"
#include <iostream>
#include <cstdlib>

void lavi::lang::error::internal(std::string_view message)
{
std::cerr
    << "\n"
    << "LAVI LANGUAGE INTERNAL ERROR\n"
    << "============================\n\n"
    << message << "\n\n"
    << "An internal error occurred while executing the program.\n"
    << "This indicates a bug in the Lavi Language implementation.\n\n"
    << "If possible, please create a minimal reproducible example\n"
    << "that triggers this error and report it at:\n\n"
    << "https://github.com/Moonslate/lavi/issues\n\n";

    std::abort();
}