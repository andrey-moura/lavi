#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <lavi/lang/api.hpp>
#include <lavi/lang/extension.hpp>

extern void create_net_class();
extern void create_http_class();
extern void create_response_class();

extern std::shared_ptr<lavi::lang::klass> net_class;
extern std::shared_ptr<lavi::lang::klass> http_class;
extern std::shared_ptr<lavi::lang::klass> response_class;

class net_extension : public lavi::lang::extension
{
public:
    net_extension() : lavi::lang::extension("net")
    {
    }

    public:
    virtual void load(lavi::lang::interpreter* interpreter) override
    {
        if(!net_class) {
            create_net_class();
        }
        if(!http_class) {
            create_http_class();
        }
        if(!response_class) {
            create_response_class();
        }

        interpreter->load(net_class);
        interpreter->load(http_class);
        interpreter->load(response_class);
    }

    virtual std::vector<std::string_view> define() override
    {
        return { "Net", "Net::HTTP", "Net::Response" };
    }
};

std::shared_ptr<lavi::lang::extension> create_net_extension()
{
    return std::make_shared<net_extension>();
}