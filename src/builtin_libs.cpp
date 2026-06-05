#include <memory>

#include "lavi/lang/extension.hpp"

extern std::shared_ptr<lavi::lang::extension> create_net_extension();
extern std::shared_ptr<lavi::lang::extension> create_yaml_extension();

void create_builtin_libs() {
    lavi::lang::extension::add_builtin(create_net_extension());
    lavi::lang::extension::add_builtin(create_yaml_extension());
}