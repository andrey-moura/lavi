#include <lavi/lang/lang.hpp>
#include <lavi/lang/api.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/error.hpp>

std::shared_ptr<lavi::lang::structure> create_hash_class(lavi::lang::interpreter* interpreter)
{
    auto HashClass = std::make_shared<lavi::lang::structure>("Hash");

        HashClass->instance_functions["present?"] = std::make_shared<lavi::lang::function>("present?", [](lavi::lang::interpreter* interpreter) {
            const auto& value = interpreter->current_context->self->as<lavi::lang::hash>();

            if(value.empty()) {
                return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
        });

        HashClass->instance_functions["[]"] = std::make_shared<lavi::lang::function>("[]", std::initializer_list<std::string>{"key"}, [](lavi::lang::interpreter* interpreter) {
            std::shared_ptr<lavi::lang::object> key = interpreter->current_context->positional_params[0];

            auto& hash = interpreter->current_context->self->as<lavi::lang::hash>();

            auto obj = hash.get(key);

            return obj;
        });

        HashClass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
            std::string result = "{";
            auto& hash = interpreter->current_context->self->as<lavi::lang::hash>();
            result.reserve(hash.keys.size() * 25);
            for(auto& key : hash.keys) {
                if(result.size() > 1) {
                    result += ", ";
                }
                auto value = hash.get(key);
                result += lavi::lang::api::call(interpreter, "to_string", key)->as<std::string>();
                result += ": ";
                result += lavi::lang::api::call(interpreter, "inspect", value)->as<std::string>();
            }
            result += "}";
            return lavi::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(result));
        });

    HashClass->instance_functions["missing"] = std::make_shared<lavi::lang::function>("missing", std::initializer_list<std::string>{"name", "aargs", "kwargs"}, [](lavi::lang::interpreter* interpreter) {
        auto it = interpreter->current_context->self->as<lavi::lang::hash>().get(interpreter->current_context->positional_params[0]);
        return it;
    });

    return HashClass;
}