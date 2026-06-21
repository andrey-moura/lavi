#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <lavi/lang/api.hpp>
#include <lavi/lang/extension.hpp>

extern std::shared_ptr<lavi::lang::klass> create_yaml_class();

class yaml_extension : public lavi::lang::extension
{
public:
    yaml_extension() : lavi::lang::extension("yaml")
    {

    }

    std::vector<std::string_view> define() override
    {
        return { "YAML" };
    }
public:
    virtual void load(lavi::lang::interpreter* interpreter) override
    {
        auto yaml_class = create_yaml_class();
        interpreter->load(yaml_class);
    }
};

std::shared_ptr<lavi::lang::extension> create_yaml_extension()
{
    return std::make_shared<yaml_extension>();
}