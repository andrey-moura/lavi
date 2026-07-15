#include <lavi/lang/lang.hpp>
#include <lavi/lang/api.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/error.hpp>

void create_array_class()
{
    lavi::lang::array_class = lavi::lang::klass::create_builtin("Array");

    lavi::lang::array_class->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
        std::string result = "[";

        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        for(auto& item : items) {
            if(result.size() > 1) {
                result += ", ";
            }

            result += lavi::lang::api::call(interpreter, "inspect", item)->as<std::string>();
        }

        result += "]";

        return lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, std::move(result));
    });

        lavi::lang::array_class->instance_functions["join"] = std::make_shared<lavi::lang::function>("join", std::vector<std::string>{"separator"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& separator = interpreter->current_context->positional_params[0]->as<std::string>();
            std::string result;

            std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

            for(auto& item : items) {
                if(result.size()) {
                    result += separator;
                }

                result += lavi::lang::api::call(interpreter, "to_string", item)->as<std::string>();
            }

            return lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, std::move(result));
        });

        lavi::lang::array_class->instance_functions["front"] = std::make_shared<lavi::lang::function>("front", [](lavi::lang::interpreter* interpreter) {
            std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

            if(items.empty()) {
                return std::make_shared<lavi::lang::object>(lavi::lang::null_class);
            }

            return items.front();
        });

        lavi::lang::array_class->instance_functions["reserve"] = std::make_shared<lavi::lang::function>("reserve", std::vector<std::string>{"new_capacity"}, [](lavi::lang::interpreter* interpreter) {
            std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();
            size_t new_capacity = interpreter->current_context->positional_params[0]->as<int>();
            items.reserve(new_capacity);
            return nullptr;
        });

        lavi::lang::array_class->instance_functions["size"] = std::make_shared<lavi::lang::function>("size", [](lavi::lang::interpreter* interpreter) {
            std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

            return lavi::lang::api::to_object(interpreter, (int)items.size());
        });

    lavi::lang::array_class->instance_functions["push"] = std::make_shared<lavi::lang::function>("push", std::initializer_list<std::string>{"item"}, [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        items.push_back(interpreter->current_context->positional_params[0]->native_copy());

    return nullptr;
    });

    lavi::lang::array_class->instance_functions["pop"] = std::make_shared<lavi::lang::function>("pop", [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        if(items.empty()) {
            return std::make_shared<lavi::lang::object>(lavi::lang::null_class);
        }

            auto item = items.back();
        items.pop_back();

        return item;
    });

    lavi::lang::array_class->instance_functions["empty?"] = std::make_shared<lavi::lang::function>("empty?", [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        bool is_empty = items.empty();

        return lavi::lang::api::to_object(interpreter, is_empty);
    });

    lavi::lang::array_class->instance_functions["include?"] = std::make_shared<lavi::lang::function>("include?", std::initializer_list<std::string>{"item"}, [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        auto& item = interpreter->current_context->positional_params[0];

        for(auto& i : items) {
            auto result = lavi::lang::api::call(interpreter, "==", item, { i });
            if(lavi::lang::api::is_truthy(interpreter, result)) {
                return lavi::lang::api::to_object(interpreter, true);
            }
        }

        return lavi::lang::api::to_object(interpreter, false);
    });

    lavi::lang::array_class->instance_functions["[]"] = std::make_shared<lavi::lang::function>("[]", std::initializer_list<std::string>{"index"}, [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        auto index = interpreter->current_context->positional_params[0]->as<int>();

        return items[index];
    });

    lavi::lang::array_class->instance_functions["=="] = std::make_shared<lavi::lang::function>("==", std::initializer_list<std::string>{"other"}, [](lavi::lang::interpreter* interpreter) {
            std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();
            if(interpreter->current_context->positional_params[0]->klass != lavi::lang::array_class) {
                return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
            }
            auto& other_items = interpreter->current_context->positional_params[0]->as<std::vector<std::shared_ptr<lavi::lang::object>>>();
            if(items.size() != other_items.size()) {
                return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
            }
            for(size_t i = 0; i < other_items.size(); ++i) {
                auto result = lavi::lang::api::call(interpreter, "==", items[i], { other_items[i] });
                if(result->klass == lavi::lang::false_class) {
                    return result;
                }
            }
            return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
        });

    lavi::lang::array_class->instance_functions["each"] = std::make_shared<lavi::lang::function>("each", [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        for(auto& item : items) {
            lavi::lang::api::yield(interpreter, { item } );
        }

        return nullptr;
    });

    lavi::lang::array_class->instance_functions["map"] = std::make_shared<lavi::lang::function>("map", [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        std::vector<std::shared_ptr<lavi::lang::object>> new_items;
        new_items.reserve(items.size());

        for(auto& item : items) {
            auto result = lavi::lang::api::yield(interpreter, { item } );

            new_items.push_back(result);
        }

        return lavi::lang::object::instantiate(interpreter, lavi::lang::array_class, std::move(new_items));
    });

    lavi::lang::array_class->instance_functions["map!"] = std::make_shared<lavi::lang::function>("map!", [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();

        for(size_t i = 0; i < items.size(); ++i) {
            auto& item = items[i];

            auto result = lavi::lang::api::yield(interpreter, { item } );

            items[i] = result;
        }

        return interpreter->current_context->self->shared_from_this();
    });

    lavi::lang::array_class->instance_functions["sort"] = std::make_shared<lavi::lang::function>("sort", [](lavi::lang::interpreter* interpreter) {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();
        std::vector<std::shared_ptr<lavi::lang::object>> sorted_items = items;
        std::sort(sorted_items.begin(), sorted_items.end(),
            [interpreter](const std::shared_ptr<lavi::lang::object> & a, const std::shared_ptr<lavi::lang::object> & b) {
                auto result = lavi::lang::api::call(interpreter, "<", a, { b });
                return lavi::lang::api::is_truthy(interpreter, result);
            });

        return lavi::lang::api::to_object(interpreter, std::move(sorted_items));
    });

    lavi::lang::array_class->instance_functions["find"] = std::make_shared<lavi::lang::function>("find", [](lavi::lang::interpreter* interpreter) -> std::shared_ptr<lavi::lang::object> {
        std::vector<std::shared_ptr<lavi::lang::object>>& items = interpreter->current_context->self->as<std::vector<std::shared_ptr<lavi::lang::object>>>();
        for(auto& item : items) {
            auto result = lavi::lang::api::yield(interpreter, { item } );
            if(lavi::lang::api::is_truthy(interpreter, result)) {
                return item;
            }
        }

        return nullptr;
    });
}