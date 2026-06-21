#include "lavi/lang/yaml.hpp"
#include "lavi/lang/hash.hpp"

#include "lavi/lang/api.hpp"

std::shared_ptr<lavi::lang::klass> create_yaml_class()
{
    auto yaml_class = lavi::lang::klass::create_builtin("YAML");

    yaml_class->functions["parse"] = std::make_shared<lavi::lang::function>("parse", std::initializer_list<std::string>{ "what" }, [yaml_class](lavi::lang::interpreter* interpreter) {
        std::string content;

        auto& what_object = interpreter->current_context->positional_params[0];

        if(what_object->klass == lavi::lang::path_class) {
            content = lavi::lang::api::call(interpreter, "File.read", { what_object })->as<std::string>();
        } else if(what_object->klass == lavi::lang::string_class) {
            content = what_object->as<std::string>();
        } else {
            content = lavi::lang::api::call(interpreter, "to_string", what_object)->as<std::string>();
        }

        auto yaml = lavi::lang::yaml::parse(content);

        lavi::lang::hash hash(interpreter);

        for(const auto& value : yaml.values())
        {
            std::shared_ptr<lavi::lang::object> obj;

            switch(value.second.type)
            {
                case lavi::lang::yaml_value::yaml_value_type::string:
                    obj = lavi::lang::api::to_object(interpreter, std::string(value.second.value));
                    break;
                default:
                    throw std::runtime_error("invalid YAML value type");
                    break;
            }

            hash.set(lavi::lang::api::to_object(interpreter, value.first), std::move(obj));
        }

        return lavi::lang::api::to_object(interpreter, std::move(hash));
    });

    return yaml_class;
}