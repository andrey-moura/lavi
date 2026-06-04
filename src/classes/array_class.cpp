#include <lavi/lang/lang.hpp>
#include <lavi/lang/api.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/error.hpp>

std::shared_ptr<lavi::lang::structure> create_array_class(lavi::lang::interpreter* interpreter)
{
    auto ArrayClass = std::make_shared<lavi::lang::structure>("Array");

    ArrayClass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
        std::string result = "[";

        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        for(auto& item : items) {
            if(result.size() > 1) {
                result += ", ";
            }

            result += lavi::lang::api::call(interpreter, "to_string", item)->as<std::string>();
        }

        result += "]";

        return lavi::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(result));
    });

    ArrayClass->instance_functions["inspect"] = std::make_shared<lavi::lang::function>("inspect", [](lavi::lang::interpreter* interpreter) {
        return lavi::lang::api::call(interpreter, "to_string", interpreter->current_context->self->shared_from_this());
    });

        ArrayClass->instance_functions["join"] = std::make_shared<lavi::lang::function>("join", std::vector<std::string>{"separator"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& separator = interpreter->current_context->positional_params[0]->as<std::string>();
            std::string result;

            std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

            for(auto& item : items) {
                if(result.size()) {
                    result += separator;
                }

                result += lavi::lang::api::call(interpreter, "to_string", item)->as<std::string>();
            }

            return lavi::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(result));
        });

        ArrayClass->instance_functions["front"] = std::make_shared<lavi::lang::function>("front", [](lavi::lang::interpreter* interpreter) {
            std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

            if(items.empty()) {
                return std::make_shared<lavi::lang::object>(interpreter->NullClass);
            }

            return items.front();
        });

        ArrayClass->instance_functions["size"] = std::make_shared<lavi::lang::function>("size", [](lavi::lang::interpreter* interpreter) {
            std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

            return lavi::lang::object::instantiate(interpreter, interpreter->IntegerClass, items.size());
        });

    ArrayClass->instance_functions["push"] = std::make_shared<lavi::lang::function>("push", std::initializer_list<std::string>{"item"}, [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        items.push_back(interpreter->current_context->positional_params[0]->native_copy());

        return nullptr;
    });

    ArrayClass->instance_functions["pop"] = std::make_shared<lavi::lang::function>("pop", [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        if(items.empty()) {
            return std::make_shared<lavi::lang::object>(interpreter->NullClass);
        }

        auto item = items.back();
        items.pop_back();

        return item;
    });

    ArrayClass->instance_functions["[]"] = std::make_shared<lavi::lang::function>("[]", std::initializer_list<std::string>{"index"}, [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        auto index = interpreter->current_context->positional_params[0]->as<int>();

        return items[index];
    });

    ArrayClass->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
            std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();
            if(interpreter->current_context->positional_params[0]->cls != interpreter->ArrayClass) {
                return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
            }
            auto& other_items = interpreter->current_context->positional_params[0]->as<std::vector<std::shared_ptr<lavi::lang::object>>>();
            if(items.size() != other_items.size()) {
                return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
            }
            for(size_t i = 0; i < other_items.size(); ++i) {
                auto it = items[i]->cls->instance_functions.find("==");
                if(it == items[i]->cls->instance_functions.end()) {
                    throw std::runtime_error("class " + std::string(items[i]->cls->name) + " does not have a method '=='");
                }
                lavi::lang::function_call call{
                    "==",
                    items[i]->cls,
                    items[i],
                    it->second.get(),
                    { other_items[i] }
                };
                auto result = lavi::lang::api::call(interpreter, "==", items[i], { other_items[i] });
                if(result->cls == interpreter->FalseClass) {
                    return result;
                }
            }
            return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
        });

    ArrayClass->instance_functions["each"] = std::make_shared<lavi::lang::function>("each", [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        for(auto& item : items) {
            lavi::lang::api::yield(interpreter, { item } );
        }

        return nullptr;
    });

    ArrayClass->instance_functions["map"] = std::make_shared<lavi::lang::function>("map", [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        std::vector<std::shared_ptr<lavi::lang::object>> new_items;
        new_items.reserve(items.size());

        for(auto& item : items) {
            auto result = lavi::lang::api::yield(interpreter, { item } );

            new_items.push_back(result);
        }

        return lavi::lang::object::instantiate(interpreter, interpreter->ArrayClass, std::move(new_items));
    });

    ArrayClass->instance_functions["map!"] = std::make_shared<lavi::lang::function>("map!", [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        for(size_t i = 0; i < items.size(); ++i) {
            auto& item = items[i];

            auto result = lavi::lang::api::yield(interpreter, { item } );

            items[i] = result;
        }

        return interpreter->current_context->self->shared_from_this();
    });

    return ArrayClass;
}