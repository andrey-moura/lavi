#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <lavi/lang/api.hpp>
#include <lavi/lang/extension.hpp>

extern std::shared_ptr<lavi::lang::klass> create_net_class();
extern std::shared_ptr<lavi::lang::klass> create_http_class();

class net_extension : public lavi::lang::extension
{
public:
    net_extension() : lavi::lang::extension("net")
    {
    }
public:
    virtual void load(lavi::lang::interpreter* interpreter) override
    {
        auto net_class = create_net_class();
        auto http_class = create_http_class();

        lavi::lang::api::contained_class(net_class, http_class);

        interpreter->load(net_class);
    }

    virtual std::vector<std::string_view> define() override
    {
        return { "NET" };
    }
};

std::shared_ptr<lavi::lang::extension> create_net_extension()
{
    return std::make_shared<net_extension>();
}