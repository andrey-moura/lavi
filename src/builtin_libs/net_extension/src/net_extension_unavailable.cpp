#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "lavi/lang/api.hpp"
#include "lavi/lang/extension.hpp"

class net_extension : public lavi::lang::extension
{
public:
    net_extension()
        : lavi::lang::extension("net")
    {
    }
protected:
    std::shared_ptr<lavi::lang::object> application_instance;
public:
    virtual void load(lavi::lang::interpreter* interpreter) override
    {
        throw std::runtime_error("This distribution of Andy was built without networking support. Please, recompile it with the networking support or download the official release.");
    }
};

std::shared_ptr<lavi::lang::extension> create_net_extension()
{
    return std::make_shared<net_extension>();
}