#include <vector>
#include <map>
#include <string>
#include <string_view>
#include <memory>

namespace lavi
{
    namespace lang
    {
        class object;
        struct yaml_value
        {
            std::string_view value;
            enum class yaml_value_type {
                string
            } type;
        };
        class yaml
        {
        public:
            static yaml parse(std::string_view source);
            static yaml parse(std::string source);
        private:
            std::shared_ptr<std::string> m_source;
            std::string_view m_source_view;
            std::map<std::string_view, yaml_value> m_values;
        public:
            const std::map<std::string_view, yaml_value>& values() { return m_values; }
        };
    }
};