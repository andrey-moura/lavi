#include <andy/lang/lang.hpp>
#include <andy/lang/api.hpp>
#include <andy/lang/interpreter.hpp>
#include <andy/lang/error.hpp>

std::shared_ptr<andy::lang::structure> create_hash_class(andy::lang::interpreter* interpreter)
{
    auto HashClass = std::make_shared<andy::lang::structure>("Hash");

        HashClass->instance_functions["present?"] = std::make_shared<andy::lang::function>("present?", [](andy::lang::interpreter* interpreter) {
            const auto& value = interpreter->current_context->self->as<andy::lang::hash>();

            if(value.empty()) {
                return std::make_shared<andy::lang::object>(interpreter->FalseClass);
            }

            return std::make_shared<andy::lang::object>(interpreter->TrueClass);
        });

        HashClass->instance_functions["[]"] = std::make_shared<andy::lang::function>("[]", std::initializer_list<std::string>{"key"}, [](andy::lang::interpreter* interpreter) {
            std::shared_ptr<andy::lang::object> key = interpreter->current_context->positional_params[0];

            auto& hash = interpreter->current_context->self->as<andy::lang::hash>();

            auto obj = hash.get(key);

            return obj;
        });

        HashClass->instance_functions["to_string"] = std::make_shared<andy::lang::function>("to_string", [](andy::lang::interpreter* interpreter) {
            std::string result = "{";
            auto& hash = interpreter->current_context->self->as<andy::lang::hash>();
            for(auto& key : hash.keys) {
                auto value = hash.get(key);
                result += andy::lang::api::call<std::string>(interpreter, andy::lang::function_call{
                    "to_string",
                    key->cls,
                    key,
                    key->cls->instance_functions["to_string"].get(),
                    {},
                    {},
                    nullptr
                });
                result += ": ";
                result += andy::lang::api::call<std::string>(interpreter, andy::lang::function_call{
                    "to_string",
                    value->cls,
                    value,
                    value->cls->instance_functions["to_string"].get(),
                    {},
                    {},
                    nullptr
                });
                result += ", ";
            }
            result += "}";
            return andy::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(result));
        });

    
    return HashClass;
}