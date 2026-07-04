#include <lavi/lang/api.hpp>

std::shared_ptr<lavi::lang::klass> net_class = nullptr;

void create_net_class()
{
    net_class = lavi::lang::klass::create_builtin("Net");
}