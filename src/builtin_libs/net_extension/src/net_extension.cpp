#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <lavi/lang/api.hpp>
#include <lavi/lang/extension.hpp>

extern std::shared_ptr<lavi::lang::structure> create_net_class(lavi::lang::interpreter* interpreter);
extern std::shared_ptr<lavi::lang::structure> create_http_class(lavi::lang::interpreter* interpreter);

class net_extension : public lavi::lang::extension
{
public:
    net_extension() : lavi::lang::extension("net")
    {
    }
public:
    virtual void load(lavi::lang::interpreter* interpreter) override
    {
        auto net_class = create_net_class(interpreter);
        auto http_class = create_http_class(interpreter);

        lavi::lang::api::contained_class(interpreter, net_class, http_class);

        interpreter->load(net_class);
    }
};

std::shared_ptr<lavi::lang::extension> create_net_extension()
{
    return std::make_shared<net_extension>();
}