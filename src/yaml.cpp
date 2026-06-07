#include "lavi/lang/yaml.hpp"

lavi::lang::yaml lavi::lang::yaml::parse(std::string_view source)
{
    yaml result;
    result.m_source_view = source;

    auto discard_whitespaces = [&source]() {
        while(source.size() && std::isspace(source.front())) {
            source.remove_prefix(1);
        }
    };

    while(source.size()) {
        discard_whitespaces();

        if(source.front() == '#') {
            do {
                source.remove_prefix(1);
            } while(source.size() && source.front() != '\n');

            if(source.size()) {
                // \n
                source.remove_prefix(1);
            }

            continue;
        }

        const char* key_start = source.data();

        while(source.size() && source.front() != ':')
        {
            source.remove_prefix(1);
        }

        const char* key_end = source.data();

        std::string_view key(key_start, key_end - key_start);

        source.remove_prefix(1);

        discard_whitespaces();

        const char* value_start = source.data();

        while(source.size() && source.front() != '\n')
        {
            source.remove_prefix(1);
        }

        const char* value_end = source.data();

        std::string_view value(value_start, value_end - value_start);

        if(source.size() && source.front() == '\n') {
            source.remove_prefix(1);
        }

        yaml_value value_obj;
        value_obj.value = value;
        value_obj.type = yaml_value::yaml_value_type::string;

        result.m_values[key] = std::move(value_obj);
    }

    return result;
}

lavi::lang::yaml lavi::lang::yaml::parse(std::string source)
{
    std::shared_ptr<std::string> source_ptr = std::make_shared<std::string>(std::move(source));

    yaml result = parse(std::string_view(*source_ptr));

    // Keeps the source alive
    result.m_source = source_ptr;

    return result;
}