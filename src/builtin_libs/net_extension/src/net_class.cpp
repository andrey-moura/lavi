#include <lavi/lang/api.hpp>

std::shared_ptr<lavi::lang::klass> create_net_class()
{
    auto net_class = lavi::lang::klass::create_builtin("Net");
    return net_class;
}