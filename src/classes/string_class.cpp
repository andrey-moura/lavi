#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/api.hpp>
#include <lavi/lang/exception.hpp>

static bool utf8_is_multibyte_character_continuation(const char& c)
{
    return ((uint8_t)c & 0b11000000) == 0b10000000;
}

void create_string_class()
{
    lavi::lang::string_class = lavi::lang::klass::create_builtin("String");

    lavi::lang::string_class->instance_functions["inspect"] = std::make_shared<lavi::lang::function>("inspect", [](lavi::lang::interpreter* interpreter) {
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
        return lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, std::move(result));
    });

    lavi::lang::string_class->instance_functions["hash"] = std::make_shared<lavi::lang::function>("hash", [](lavi::lang::interpreter* interpreter) {
        const std::string& value = interpreter->current_context->self->as<std::string>();
        size_t hash_value = std::hash<std::string>{}(value);
        return lavi::lang::object::instantiate(interpreter, lavi::lang::integer_class, (int)hash_value);
    });

    lavi::lang::string_class->instance_functions["*"] = std::make_shared<lavi::lang::function>("*", std::initializer_list<std::string>{"what"}, [](lavi::lang::interpreter* interpreter) {
        const std::string& value = interpreter->current_context->self->as<std::string>();
        const auto& params = interpreter->current_context->positional_params;
        std::string result;

        if(params[0]->klass != lavi::lang::integer_class) {
            throw std::runtime_error("undefined operator * (" + std::string(interpreter->current_context->self->klass->name) + ", " + std::string(params[0]->klass->name) + ")");
        }
   
        int times = params[0]->as<int>();

        for(int i = 0; i < times; i++) {
            result += value;
        }

        return lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, result);
    });

    lavi::lang::string_class->instance_functions["[]"] = std::make_shared<lavi::lang::function>("[]", std::initializer_list<std::string>{"what"}, [](lavi::lang::interpreter* interpreter) {
        const std::string& value = interpreter->current_context->self->as<std::string>();
        const auto& params = interpreter->current_context->positional_params;

        if(params[0]->klass != lavi::lang::integer_class) {
            throw std::runtime_error("undefined operator [] (" + std::string(interpreter->current_context->self->klass->name) + ", " + std::string(params[0]->klass->name) + ")");
        }

        int index = params[0]->as<int>();

        if(index < 0 || (size_t)index >= value.size()) {
            return std::make_shared<lavi::lang::object>(lavi::lang::null_class);
        }

        return lavi::lang::api::to_object(interpreter, std::move(std::string(1, value[index])));
    });

    lavi::lang::string_class->instance_functions["+="] = std::make_shared<lavi::lang::function>("+=", std::initializer_list<std::string>{"what"}, [](lavi::lang::interpreter* interpreter) {
        std::string& value = interpreter->current_context->self->as<std::string>();
        const auto& params = interpreter->current_context->positional_params;
        value += params[0]->as<std::string>();
        return interpreter->current_context->self; 
    });

        lavi::lang::string_class->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();

            if(value.empty()) {
                return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
            }

            return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
        });

        lavi::lang::string_class->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            return lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, value);
        });

        lavi::lang::string_class->instance_functions["find"] = std::make_shared<lavi::lang::function>("find", std::initializer_list<std::string>{"what"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            size_t pos = value.find(interpreter->current_context->positional_params[0]->as<std::string>());
            return lavi::lang::object::instantiate(interpreter, lavi::lang::integer_class, (int32_t)pos);
        });

        lavi::lang::string_class->instance_functions["slice"] = std::make_shared<lavi::lang::function>("substring", std::initializer_list<std::string>{"start", "size"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const auto& params = interpreter->current_context->positional_params;
            size_t start = params[0]->as<int32_t>();
            size_t size = params[1]->as<int32_t>();

            return lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, value.substr(start, size));
        });

        lavi::lang::string_class->instance_functions["slice!"] = std::make_shared<lavi::lang::function>("slice!", std::initializer_list<std::string>{"start", "size"}, [](lavi::lang::interpreter* interpreter) {
            std::string& value = interpreter->current_context->self->as<std::string>();
            const auto& params = interpreter->current_context->positional_params;
            size_t start = params[0]->as<int32_t>();
            size_t size = params[1]->as<int32_t>();

            auto copy = value.substr(start, size);

            value.erase(start, size);

            return lavi::lang::api::to_object(interpreter, std::move(copy));
        });

        lavi::lang::string_class->instance_functions["to_lower_case!"] = std::make_shared<lavi::lang::function>("to_lower_case!", [](lavi::lang::interpreter* interpreter) {
            std::string& value = interpreter->current_context->self->as<std::string>();

            for(char & c : value) {
                c = std::tolower(c);
            }

            return nullptr;
        });

        lavi::lang::string_class->instance_functions["to_lower_case"] = std::make_shared<lavi::lang::function>("to_lower_case!", [](lavi::lang::interpreter* interpreter) {
            std::string value = interpreter->current_context->self->as<std::string>();

            for(char & c : value) {
                c = std::tolower(c);
            }

            return lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, value);
        });

        lavi::lang::string_class->instance_functions["to_integer"] = std::make_shared<lavi::lang::function>("to_integer", [](lavi::lang::interpreter* interpreter) {
            std::string value = interpreter->current_context->self->as<std::string>();

            if(value.empty()) return std::make_shared<lavi::lang::object>(lavi::lang::null_class);

            if(!isdigit(value[0])) return std::make_shared<lavi::lang::object>(lavi::lang::null_class);

            size_t pos = 0;
            int result = std::stoi(value, &pos);

            if(pos != value.size()) return std::make_shared<lavi::lang::object>(lavi::lang::null_class);

            return lavi::lang::object::instantiate(interpreter, lavi::lang::integer_class, result);
        });

        lavi::lang::string_class->instance_functions["erase!"] = std::make_shared<lavi::lang::function>("erase!", std::initializer_list<std::string>{"start", "size"}, [](lavi::lang::interpreter* interpreter) {
            std::string& value = interpreter->current_context->self->as<std::string>();
            const auto& params = interpreter->current_context->positional_params;
            size_t start = params[0]->as<int32_t>();
            size_t size = params[1]->as<int32_t>();

            value.erase(start, size);

            return nullptr;
        });

    lavi::lang::string_class->instance_functions["starts_with?"] = std::make_shared<lavi::lang::function>("starts_with?", std::initializer_list<std::string>{"what"}, [](lavi::lang::interpreter* interpreter) {
        std::string& value = interpreter->current_context->self->as<std::string>();
        const std::string& what = interpreter->current_context->positional_params[0]->as<std::string>();

        bool starts = value.starts_with(what);

        return lavi::lang::api::to_object(interpreter, starts);
    });

    lavi::lang::string_class->instance_functions["ends_with?"] = std::make_shared<lavi::lang::function>("ends_with?", std::initializer_list<std::string>{"what"}, [](lavi::lang::interpreter* interpreter) {
        std::string& value = interpreter->current_context->self->as<std::string>();
        const std::string& what = interpreter->current_context->positional_params[0]->as<std::string>();

        bool ends = value.ends_with(what);

        return lavi::lang::api::to_object(interpreter, ends);
    });

        lavi::lang::string_class->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const std::string& other = interpreter->current_context->positional_params[0]->as<std::string>();

            return lavi::lang::api::to_object(interpreter, value == other);
        });

        lavi::lang::string_class->instance_functions["!="] = std::make_shared<lavi::lang::function>("!=", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const std::string& other = interpreter->current_context->positional_params[0]->as<std::string>();

            if(value != other) {
                return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
            }

            return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
        });

        lavi::lang::string_class->instance_functions["+"] = std::make_shared<lavi::lang::function>("+", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const std::string& other = interpreter->current_context->positional_params[0]->as<std::string>();

            return lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, value + other);
        });

        lavi::lang::string_class->instance_functions["<"] = std::make_shared<lavi::lang::function>("<", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
            if(interpreter->current_context->positional_params[0]->klass != lavi::lang::string_class) {
                throw lavi::lang::exception(
                    interpreter,
                    "Cannot compare String with " + interpreter->current_context->positional_params[0]->klass->name,
                    lavi::lang::runtime_error_class
                );
            }
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const std::string& other = interpreter->current_context->positional_params[0]->as<std::string>();

            return lavi::lang::api::to_object(interpreter, value < other);
        });

        lavi::lang::string_class->instance_functions["size"] = std::make_shared<lavi::lang::function>("size", [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();

            int size = 0;

            for(char c : value) {
                if(utf8_is_multibyte_character_continuation(c)) {
                    continue;
                }

                size++;
            }

            return lavi::lang::api::to_object(interpreter, size);
        });

        lavi::lang::string_class->instance_functions["empty?"] = std::make_shared<lavi::lang::function>("empty?", [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();

            if(value.empty()) {
                return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
            }

            return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
        });

        lavi::lang::string_class->instance_functions["include?"] = std::make_shared<lavi::lang::function>("include?", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& value = interpreter->current_context->self->as<std::string>();
            const std::string& other = interpreter->current_context->positional_params[0]->as<std::string>();

            if(value.find(other) != std::string::npos) {
                return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
            }

            return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
        });

        lavi::lang::string_class->instance_functions["capitalize!"] = std::make_shared<lavi::lang::function>("capitalize!", [](lavi::lang::interpreter* interpreter) {
            std::string& value = interpreter->current_context->self->as<std::string>();

            if(!value.empty()) {
                value[0] = std::toupper(value[0]);
            }

            return nullptr;
        });

  lavi::lang::string_class->instance_functions["front"] = std::make_shared<lavi::lang::function>("front", [](lavi::lang::interpreter* interpreter) {
    const std::string& value = interpreter->current_context->self->as<std::string>();
    std::string result;
    result.push_back(value.front());

    return lavi::lang::api::to_object(interpreter, result);
  });

  lavi::lang::string_class->instance_functions["pop_front"] = std::make_shared<lavi::lang::function>("pop_front", [](lavi::lang::interpreter* interpreter) {
    std::string& value = interpreter->current_context->self->as<std::string>();
    value.erase(value.begin());

    return nullptr;
  });
}