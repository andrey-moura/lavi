#include <memory>

#include "andy/lang/extension.hpp"

extern std::shared_ptr<andy::lang::extension> create_net_extension();

void create_builtin_libs() {
    andy::lang::extension::add_builtin(create_net_extension());
}