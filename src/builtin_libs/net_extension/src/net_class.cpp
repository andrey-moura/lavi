#include <lavi/lang/api.hpp>

std::shared_ptr<lavi::lang::structure> create_net_class(lavi::lang::interpreter* interpreter)
{
    auto net_class = std::make_shared<lavi::lang::structure>("Net");
    return net_class;
}