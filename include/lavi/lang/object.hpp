#pragma once

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "lavi/lang/function.hpp"
#include "lavi/lang/class.hpp"
#include "lavi/lang/scope.hpp"

namespace lavi
{
    namespace lang {
        class object;
        class klass;
        class interpreter;
        constexpr size_t max_native_size = 40;
        class object : public std::enable_shared_from_this<object>, public scope
        {
        public:
            object(std::shared_ptr<lavi::lang::klass> c);
            ~object();
        public:
            const std::string& default_string_representation();
        public:
            std::shared_ptr<lavi::lang::klass> klass;
            std::shared_ptr<lavi::lang::object> base_instance = nullptr;
            std::shared_ptr<lavi::lang::object> derived_instance = nullptr;
            // #ifdef __ANDY_DEBUG__
            // lavi::lang::object* debug_object = this;

            // __attribute__((noinline)) __attribute__((used)) std::string debug_string()
            // {
            //     return to_var().to_s();
            // }
            // #endif
        protected:
            // A pointer to the native object
            void* native_ptr = nullptr;
            // The native object
            uint8_t native[max_native_size] = {0};
        private:
            // The native object destructor function ptr.
            void (*native_destructor)(object* obj) = nullptr;
            // The native move function ptr.
            void (*native_move_ptr)(object* dest, object* src) = nullptr;
            // The native object copy function ptr.
            std::shared_ptr<object> (*native_copy_ptr)(lavi::lang::object* obj) = nullptr;
#ifdef __ANDY_DEBUG__
            int* native_int = (int*)native;
            std::string* native_string = (std::string*)native;
#endif
            std::string string_representation_cache;
        public:
            void initialize(lavi::lang::interpreter* interpreter);
        public:
            /// @brief Initialize the object with a value.
            /// @param klass The class of the object.
            /// @return Returns a shared pointer to the object.
            static auto instantiate(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::klass> klass)
            {
                auto obj = std::make_shared<lavi::lang::object>(klass);
                obj->initialize(interpreter);

                return obj;
            }
            /// @brief Initialize the object with a value.
            /// @tparam T The type of the value.
            /// @param klass The class of the object.
            /// @param value The pointer to the value. This will be deleted when the object is destroyed.
            /// @return Returns a shared pointer to the object.
            template<typename T>
            static std::shared_ptr<lavi::lang::object> instantiate(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::klass> klass, T* value)
            {
                auto obj = std::make_shared<lavi::lang::object>(klass);
                obj->set_native_ptr<T>(obj.get(), value);

                obj->initialize(interpreter);

                return obj;
            }
            /// @brief Initialize the object with a value.
            /// @tparam T The type of the value.
            /// @param klass The class of the object.
            /// @param value The value.
            /// @return Returns a shared pointer to the object.
            template<typename T>
            static std::enable_if<!std::is_pointer<T>::value, std::shared_ptr<lavi::lang::object>>::type instantiate(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::klass> klass, T value)
            {
                auto obj = std::make_shared<lavi::lang::object>(klass);

                if(!std::is_same_v<T, std::nullptr_t>) {
                    obj->set_native<T>(std::move(value));
                }

                obj->initialize(interpreter);

                return obj;
            }
            static std::shared_ptr<lavi::lang::object> create(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::klass> klass)
            {
                auto obj = std::make_shared<lavi::lang::object>(klass);
                obj->initialize(interpreter);
                return obj;
            }
            /// @brief Creates the object with a value.
            /// @tparam T The type of the value.
            /// @param klass The class of the object.
            /// @param value The value.
            /// @return Returns a shared pointer to the object.
            template<typename T>
            static std::enable_if<!std::is_pointer<T>::value, std::shared_ptr<lavi::lang::object>>::type create(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::klass> klass, T value)
            {
                auto obj = std::make_shared<lavi::lang::object>(klass);
                obj->set_native<T>(std::move(value));
                obj->initialize(interpreter);

                return obj;
            }
            /// @brief Creates the object with a pointer to a value.
            /// @tparam T The type of the value.
            /// @param value The pointer to the value.
            /// @return Returns a shared pointer to the object.
            template<typename T>
            static std::shared_ptr<lavi::lang::object> create(lavi::lang::interpreter* interpreter, std::shared_ptr<lavi::lang::klass> klass, T* value)
            {
                auto obj = std::make_shared<lavi::lang::object>(klass);
                obj->set_native_ptr(value);
                return obj;
            }
            template<typename T>
            void set_native(T value) {
                if(native_destructor) {
                    native_destructor(this);
                }

                bool should_destroy = false;

                // Under GCC, even if the constexpr is false, the code still generates warning when 
                // sizoef(T) > max_native_size. So we need silence the warning.
                if constexpr(sizeof(T) <= max_native_size) {
                    if constexpr(std::is_arithmetic<T>::value) {
                        // Boolean, integer, float, etc.
                        *((T*)(&this->native)) = value;
                    } else {
                        new ((T*)(&this->native)) T(std::move(value));
                        should_destroy = true;
                    }
                } else {
                    this->native_ptr = new T(std::move(value));
                    should_destroy = true;
                }
                set_destructor<T>(this);
            }

            template<typename T>
            void set_native_ptr(T* ptr) {
                this->native_ptr = (void*)ptr;
                set_destructor<T>(this);
            }

            std::shared_ptr<object> native_copy() {
                if(native_copy_ptr) {
                    return native_copy_ptr(this);
                }

                auto obj = std::make_shared<lavi::lang::object>(klass);
                for(const auto& [var_name, var_value] : variables) {
                    obj->variables[var_name] = var_value->native_copy();
                }
                obj->functions = functions;
                obj->inline_functions = inline_functions;
                if(base_instance) {
                    obj->base_instance = base_instance->native_copy();
                }
                if(derived_instance) {
                    obj->derived_instance = derived_instance->native_copy();
                }
                return obj;
            }
        private:
            template <typename T>
            static void set_destructor(object* obj) {
                if constexpr(!std::is_arithmetic<T>::value) {
                    obj->native_destructor = [](object* obj) {
                        obj->log_native_destructor();

                        if(obj->native_ptr) {
                            delete (T*)obj->native_ptr;
                        } else {
                            ((T*)(&obj->native))->~T();
                        }
                    };
                }

                obj->native_copy_ptr = [](lavi::lang::object* obj) -> std::shared_ptr<lavi::lang::object> {
                    std::shared_ptr<lavi::lang::object> other = std::make_shared<lavi::lang::object>(obj->klass);

                    if constexpr(std::is_copy_constructible<T>::value)
                    {
                        other->set_native<T>(obj->as<T>());
                    } else {
                        throw std::runtime_error("Type " + std::string(obj->klass->name) + " does not support copy construction");
                    }

                    for(const auto& [var_name, var_value] : obj->variables) {
                        other->variables[var_name] = var_value->native_copy();
                    }

                    other->functions = obj->functions;
                    other->inline_functions = obj->inline_functions;

                    if(obj->base_instance) {
                        other->base_instance = obj->base_instance->native_copy();
                    }
                    if(obj->derived_instance) {
                        other->derived_instance = obj->derived_instance->native_copy();
                    }

                    return other;
                };

                obj->native_move_ptr = [](object* dest, object* src) {
                    dest->klass = std::move(src->klass);
                    dest->base_instance = std::move(src->base_instance);
                    dest->derived_instance = std::move(src->derived_instance);

                    dest->variables = std::move(src->variables);
                    dest->functions = std::move(src->functions);
                    dest->inline_functions = std::move(src->inline_functions);

                    T value = std::move(src->as<T>());
                    dest->set_native<T>(std::move(value));

                    src->native_destructor = nullptr;
                    src->native_copy_ptr = nullptr;
                    src->native_ptr = nullptr;
                    std::memset(src->native, 0, max_native_size);
                };
            }

            void log_native_destructor();
        public:
            template<typename T>
            const T& as() const {
                if(base_instance) {
                    return base_instance->as<T>();
                }
                if(native_ptr) {
                    return *static_cast<T*>(native_ptr);
                }

                return *static_cast<T*>((void*)native);
            }
            template<typename T>
            T& as() {
                if(base_instance) {
                    return base_instance->as<T>();
                }
                if(native_ptr) {
                    return *static_cast<T*>(native_ptr);
                }

                return *static_cast<T*>((void*)native);
            }
            public:
                object& operator=(object&& other) noexcept
                {
                    if(other.native_move_ptr) {
                        other.native_move_ptr(this, &other);
                    } else {
                        this->klass = std::move(other.klass);
                        this->base_instance = std::move(other.base_instance);
                        this->derived_instance = std::move(other.derived_instance);
                        this->variables = std::move(other.variables);
                        this->functions = std::move(other.functions);
                        this->inline_functions = std::move(other.inline_functions);

                        other.native_destructor = nullptr;
                        other.native_copy_ptr = nullptr;
                        other.native_ptr = nullptr;
                    }
                    return *this;
                }
        };
    };
};