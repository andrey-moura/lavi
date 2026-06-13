#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/api.hpp>

std::shared_ptr<lavi::lang::structure> create_string_class(lavi::lang::interpreter* interpreter)
{
    auto StringClass = std::make_shared<lavi::lang::structure>("String");

    StringClass->instance_functions["inspect"] = std::make_shared<lavi::lang::function>("inspect", [StringClass](lavi::lang::interpreter* interpreter) {
        const std::string& value = interpreter->current_context->self->as<std::string>();
        std::string result = "\"";
        result.reserve(value.size() + (value.size() / 2) + 2); // Reserve some extra space for escaped characters
        for(char c : value) {
            switch(c) {
                case '\n': result += "\\n"; break;
                case '\t': result += "\\t"; break;
                case '\r': result += "\\r"; break;
                case '\\': result += "\\\\"; break;
                case '\"': result += "\\\""; break;
                default: result += c; break;
            }
        }
        result += "\"";
        return lavi::lang::object::instantiate(interpreter, StringClass, std::move(result));
    });

    StringClass->instance_functions["hash"] = std::make_shared<lavi::lang::function>("hash", [StringClass](lavi::lang::interpreter* interpreter) {
        const std::string& value = interpreter->current_context->self->as<std::string>();
        size_t hash_value = std::hash<std::string>{}(value);
        return lavi::lang::object::instantiate(interpreter, interpreter->IntegerClass, (int)hash_value);
    });

    StringClass->instance_functions["*"] = std::make_shared<lavi::lang::function>("*", std::initializer_list<std::string>{"what"}, [StringClass](lavi::lang::interpreter* interpreter) {
        const std::string& value = interpreter->current_context->self->as<std::string>();
        const auto& params = interpreter->current_context->positional_params;
        std::string result;

        if(params[0]->cls != interpreter->IntegerClass) {
            throw std::runtime_error("undefined operator * (" + std::string(interpreter->current_context->self->cls->name) + ", " + std::string(params[0]->cls->name) + ")");
        }
   
        int times = params[0]->as<int>();

        for(int i = 0; i < times; i++) {
            result += value;
        }

        return lavi::lang::object::instantiate(interpreter, StringClass, result);
    });

    StringClass->instance_functions["[]"] = std::make_shared<lavi::lang::function>("[]", std::initializer_list<std::string>{"what"}, [StringClass](lavi::lang::interpreter* interpreter) {
        const std::string& value = interpreter->current_context->self->as<std::string>();
        const auto& params = interpreter->current_context->positional_params;

        if(params[0]->cls != interpreter->IntegerClass) {
            throw std::runtime_error("undefined operator [] (" + std::string(interpreter->current_context->self->cls->name) + ", " + std::string(params[0]->cls->name) + ")");
        }

        int index = params[0]->as<int>();

        if(index < 0 || (size_t)index >= value.size()) {
            return std::make_shared<lavi::lang::object>(interpreter->NullClass);
        }

        return lavi::lang::api::to_object(interpreter, std::move(std::string(1, value[index])));
    });

        StringClass->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();

            if(value.empty()) {
                return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
        });

        StringClass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [StringClass](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            return lavi::lang::object::instantiate(interpreter, StringClass, value);
        });

        StringClass->instance_functions["find"] = std::make_shared<lavi::lang::function>("find", std::initializer_list<std::string>{"what"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            size_t pos = value.find(interpreter->current_context->positional_params[0]->as<std::string>());
            return lavi::lang::object::instantiate(interpreter, interpreter->IntegerClass, (int32_t)pos);
        });

        StringClass->instance_functions["substring"] = std::make_shared<lavi::lang::function>("substring", std::initializer_list<std::string>{"start", "size"}, [StringClass](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const auto& params = interpreter->current_context->positional_params;
            size_t start = params[0]->as<int32_t>();
            size_t size = params[1]->as<int32_t>();

            return lavi::lang::object::instantiate(interpreter, StringClass, value.substr(start, size));
        });

        StringClass->instance_functions["to_lower_case!"] = std::make_shared<lavi::lang::function>("to_lower_case!", [](lavi::lang::interpreter* interpreter) {
            std::string& value = interpreter->current_context->self->as<std::string>();

            for(char & c : value) {
                c = std::tolower(c);
            }

            return nullptr;
        });

        StringClass->instance_functions["to_lower_case"] = std::make_shared<lavi::lang::function>("to_lower_case!", [StringClass](lavi::lang::interpreter* interpreter) {
            std::string value = interpreter->current_context->self->as<std::string>();

            for(char & c : value) {
                c = std::tolower(c);
            }

            return lavi::lang::object::instantiate(interpreter, StringClass, value);
        });

            StringClass->instance_functions["to_integer"] = std::make_shared<lavi::lang::function>("to_integer", [](lavi::lang::interpreter* interpreter) {
                std::string value = interpreter->current_context->self->as<std::string>();

            if(value.empty()) return std::make_shared<lavi::lang::object>(interpreter->NullClass);

            if(!isdigit(value[0])) return std::make_shared<lavi::lang::object>(interpreter->NullClass);

            size_t pos = 0;
            int result = std::stoi(value, &pos);

            if(pos != value.size()) return std::make_shared<lavi::lang::object>(interpreter->NullClass);

            return lavi::lang::object::instantiate(interpreter, interpreter->IntegerClass, result);
        });

        StringClass->instance_functions["erase!"] = std::make_shared<lavi::lang::function>("erase!", std::initializer_list<std::string>{"start", "size"}, [](lavi::lang::interpreter* interpreter) {
            std::string& value = interpreter->current_context->self->as<std::string>();
            const auto& params = interpreter->current_context->positional_params;
            size_t start = params[0]->as<int32_t>();
            size_t size = params[1]->as<int32_t>();

            value.erase(start, size);

            return nullptr;
        });

    StringClass->instance_functions["starts_with?"] = std::make_shared<lavi::lang::function>("starts_with?", std::initializer_list<std::string>{"what"}, [](lavi::lang::interpreter* interpreter) {
        std::string& value = interpreter->current_context->self->as<std::string>();
        const std::string& what = interpreter->current_context->positional_params[0]->as<std::string>();

        bool starts = value.starts_with(what);

        return lavi::lang::api::to_object(interpreter, starts);
    });

    StringClass->instance_functions["ends_with?"] = std::make_shared<lavi::lang::function>("ends_with?", std::initializer_list<std::string>{"what"}, [](lavi::lang::interpreter* interpreter) {
        std::string& value = interpreter->current_context->self->as<std::string>();
        const std::string& what = interpreter->current_context->positional_params[0]->as<std::string>();

        bool ends = value.ends_with(what);

        return lavi::lang::api::to_object(interpreter, ends);
    });

        StringClass->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const std::string& other = interpreter->current_context->positional_params[0]->as<std::string>();

            return lavi::lang::api::to_object(interpreter, value == other);
        });

        StringClass->instance_functions["!="] = std::make_shared<lavi::lang::function>("!=", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const std::string& other = interpreter->current_context->positional_params[0]->as<std::string>();

            if(value != other) {
                return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
        });

        StringClass->instance_functions["+"] = std::make_shared<lavi::lang::function>("+", std::initializer_list<std::string>{"other"}, [StringClass](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const std::string& other = interpreter->current_context->positional_params[0]->as<std::string>();

            return lavi::lang::object::instantiate(interpreter, StringClass, value + other);
        });

        StringClass->instance_functions["size"] = std::make_shared<lavi::lang::function>("size", [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            return lavi::lang::object::instantiate(interpreter, interpreter->IntegerClass, (int32_t)value.size());
        });

        StringClass->instance_functions["empty?"] = std::make_shared<lavi::lang::function>("empty?", [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();

            if(value.empty()) {
                return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
        });

        StringClass->instance_functions["include?"] = std::make_shared<lavi::lang::function>("include?", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const std::string& other = interpreter->current_context->positional_params[0]->as<std::string>();

            if(value.find(other) != std::string::npos) {
                return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
            }

            return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
        });

        StringClass->instance_functions["capitalize!"] = std::make_shared<lavi::lang::function>("capitalize!", [](lavi::lang::interpreter* interpreter) {
            std::string& value = interpreter->current_context->self->as<std::string>();

            if(!value.empty()) {
                value[0] = std::toupper(value[0]);
            }

            return nullptr;
    });
    
  StringClass->instance_functions["front"] = std::make_shared<lavi::lang::function>("front", [](lavi::lang::interpreter* interpreter) {
    const std::string& value = interpreter->current_context->self->as<std::string>();
    std::string result;
    result.push_back(value.front());

    return lavi::lang::api::to_object(interpreter, result);
  });

  StringClass->instance_functions["pop_front"] = std::make_shared<lavi::lang::function>("pop_front", [](lavi::lang::interpreter* interpreter) {
    std::string& value = interpreter->current_context->self->as<std::string>();
    value.erase(value.begin());

    return nullptr;
  });

    return StringClass;
}