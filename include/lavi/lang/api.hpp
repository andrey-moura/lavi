#pragma once

#include <filesystem>

#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/config.hpp>
#include <lavi/lang/function.hpp>

namespace lavi
{
    namespace lang
    {
        namespace api
        {
            /// @brief Executes the code in a file and return the result.
            /// @param path The path to the source code.
            /// @return Returns a shared pointer to the object.
            std::shared_ptr<lavi::lang::object> evaluate(lavi::lang::interpreter* interpreter, std::filesystem::path path, int argc = 0, char** argv = nullptr);
            /// @brief Convert or cast the object to a specific type.
            /// @tparam T The type to convert to.
            /// @param interpreter The interpreter.
            /// @param object The object to convert.
            /// @return Returns a shared pointer to the object.
            template<typename T>
            T cast_object_to(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object>&& object)
            {
                if constexpr(std::is_same_v<T, std::string>) {
                    if(object->klass == lavi::lang::string_class) {
                        return object->as<std::string>();
                    }
                    throw std::runtime_error("Cannot cast " + std::string(object->klass->name) + " to string");
                } else if constexpr(std::is_same_v<T, bool>) {
                    if(object->klass == lavi::lang::true_class) {
                        return true;
                    } else if(object->klass == lavi::lang::false_class) {
                        return false;
                    }
                    throw std::runtime_error("Cannot cast " + std::string(object->klass->name) + " to bool");
                } else {
                    throw std::runtime_error("Unsupported type for to_object: " + std::string(typeid(T).name()));
                }
            }
            /// @brief Call a function.
            /// @param interpreter The interpreter.
            /// @param function_name The function name.
            /// @return Returns a shared pointer to the object.
            std::shared_ptr<lavi::lang::object> call(
                lavi::lang::interpreter* interpreter,
                std::string_view function_name,
                std::initializer_list<std::shared_ptr<lavi::lang::object>> positional_params = {},
                std::map<std::string_view, std::shared_ptr<lavi::lang::object>> named_params = {}
            );
            /// @brief Call a function.
            /// @param interpreter The interpreter.
            /// @param object The object.
            /// @param function_name The function name.
            /// @return Returns a shared pointer to the object.
            std::shared_ptr<lavi::lang::object> call(
                lavi::lang::interpreter* interpreter,
                std::string_view function_name,
                std::shared_ptr<lavi::lang::object> object,
                std::vector<std::shared_ptr<lavi::lang::object>> positional_params = {},
                std::map<std::string_view, std::shared_ptr<lavi::lang::object>> named_params = {}
            );
            /// @brief Yield a value to the caller block.
            /// @param interpreter The interpreter.
            /// @param value The value to yield.
            /// @return Returns the value sent by the caller block.
            std::shared_ptr<lavi::lang::object> yield(lavi::lang::interpreter* interpreter, std::vector<std::shared_ptr<lavi::lang::object>> position_params = {}, std::map<std::string, std::shared_ptr<lavi::lang::object>> named_params = {});
            /// @brief Creates the object with a value and automatically determines the class.
            /// @tparam T The type of the value.
            /// @param interpreter The interpreter.
            /// @param value The value.
            /// @return Returns a shared pointer to the object.
            template<typename T>
            inline std::shared_ptr<lavi::lang::object> to_object(lavi::lang::interpreter* interpreter, T value)
            {
                if constexpr(std::is_same_v<T, int>) {
                    auto obj = lavi::lang::object::instantiate(interpreter, lavi::lang::integer_class, value);
                    return obj;
                } else if constexpr(std::is_same_v<T, std::string>) {
                    auto obj = lavi::lang::object::instantiate(interpreter, lavi::lang::string_class, std::move(value));
                    return obj;
                } else if constexpr(std::is_same_v<T, double>) {
                    auto obj = lavi::lang::object::instantiate(interpreter, lavi::lang::double_class, value);
                    return obj;
                } else if constexpr(std::is_same_v<T, float>) {
                    auto obj = lavi::lang::object::instantiate(interpreter, lavi::lang::float_class, value);
                    return obj;
                } else if constexpr(std::is_same_v<T, std::vector<std::shared_ptr<lavi::lang::object>>>) {
                    auto obj = lavi::lang::object::instantiate(interpreter, lavi::lang::array_class, std::move(value));
                    return obj;
                } else if constexpr(std::is_same_v<T, lavi::lang::hash>) {
                    auto obj = lavi::lang::object::instantiate(interpreter, lavi::lang::hash_class, std::move(value));
                    return obj;
                } else if constexpr(std::is_same_v<T, std::map<std::string, std::shared_ptr<lavi::lang::object>>> || std::is_same_v<T, std::map<std::string_view, std::shared_ptr<lavi::lang::object>>>) {
                    lavi::lang::hash hash(interpreter);
                    for(auto& [key, val] : value) {
                        hash.set(to_object(interpreter, key), std::move(val));
                    }
                    auto obj = lavi::lang::object::instantiate(interpreter, lavi::lang::hash_class, std::move(hash));
                    return obj;
                } else if constexpr(std::is_same_v<T, const char*> || std::is_same_v<T, char*> || std::is_same_v<T, std::string_view>) {
                    return to_object(interpreter, std::string(value));
                }
                else if constexpr(std::is_same_v<T, std::shared_ptr<lavi::lang::klass>>) {
                    auto class_object = std::make_shared<lavi::lang::object>(lavi::lang::class_class);
                    class_object->set_native<std::shared_ptr<lavi::lang::klass>>(std::move(value));
                    class_object->initialize(interpreter);

                    return class_object;
                } else if constexpr(std::is_same_v<T, bool>) {
                    if(value) {
                        return lavi::lang::object::instantiate(interpreter, lavi::lang::true_class);
                    } else {
                        return lavi::lang::object::instantiate(interpreter, lavi::lang::false_class);
                    }
                } else if constexpr(std::is_same_v<T, std::shared_ptr<lavi::lang::function>>) {
                    auto function_object = lavi::lang::object::instantiate(
                        interpreter,
                        lavi::lang::function_class,
                        std::move(value)
                    );
                    return function_object;
                } else if constexpr(std::is_same_v<T, std::filesystem::path>) {
                    auto obj = lavi::lang::object::create(interpreter, lavi::lang::path_class, std::move(value));
                    obj->initialize(interpreter);
                    return obj;
                } else if constexpr(std::is_same_v<T, std::vector<std::string_view>>) {
                    std::vector<std::shared_ptr<lavi::lang::object>> arr;
                    for(auto& str : value) {
                        arr.push_back(to_object(interpreter, str));
                    }
                    auto obj = lavi::lang::object::instantiate(interpreter, lavi::lang::array_class, std::move(arr));
                    return obj;
                 } else if constexpr(std::is_same_v<T, std::nullptr_t>) {
                    return lavi::lang::object::instantiate(interpreter, lavi::lang::null_class);
                 }
                else {
                    throw std::runtime_error("Unsupported type for to_object: " + std::string(typeid(T).name()));
                }
            }
            /// @brief Creates a new object of a class with the given parameters.
            /// @param interpreter The interpreter.
            /// @param klass The class of the object.
            /// @param positional_params The positional parameters to pass to the constructor.
            /// @return Returns a shared pointer to the object.
            std::shared_ptr<lavi::lang::object> new_object(
                lavi::lang::interpreter* interpreter,
                std::shared_ptr<lavi::lang::klass> klass,
                std::vector<std::shared_ptr<lavi::lang::object>> positional_params = {},
                std::map<std::string_view, std::shared_ptr<lavi::lang::object>> named_params = {}
            );
            /// @brief Checks if the object is truthy (not null and not false).
            /// @param interpreter The interpreter.
            /// @param obj The object to check.
            /// @return Returns true if the object is truthy, false otherwise.
            bool is_truthy(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object> obj);
            /// @brief Determinates if the object is an instance of the class or one of its subclasses.
            /// @param interpreter The interpreter
            /// @param obj The object to check.
            /// @param klass The class to check.
            /// @return Returns true if the object is an instance of the class or one of its subclasses, false otherwise.
            bool is_a(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::object> obj, std::shared_ptr<lavi::lang::klass> klass);
            /// @brief Adds a class to another class.
            /// @param interpreter The interpreter.
            /// @param klass The class.
            /// @param contained The contained class.
            void contained_class(std::shared_ptr<lavi::lang::klass> klass, std::shared_ptr<lavi::lang::klass> contained);
            /// @brief Load the source code without executing it.
            /// @param interpreter The interpreter.
            /// @param path_or_key The path or unique key to the source code.
            /// @param source_code The code to be executed.
            lavi::lang::parser::ast_node& load(
                lavi::lang::interpreter* interpreter,
                std::string path_or_key,
                std::string source_code
            );
        };
    };
};