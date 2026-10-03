#include <chrono>

#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/api.hpp>

#define ADD_DURATION_CAST_FUNCTION(name) \
  high_resolution_duration_class->instance_functions[#name] = std::make_shared<lavi::lang::function>(#name, [high_resolution_duration_class](lavi::lang::interpreter* interpreter) { \
    auto self = interpreter->current_context->self; \
    auto duration = self->as<std::chrono::high_resolution_clock::duration>(); \
    if constexpr (std::is_same_v<std::chrono::name, std::chrono::seconds>) { \
      auto duration_as_##name = std::chrono::duration<double>(duration); \
      return lavi::lang::api::to_object(interpreter, duration_as_##name.count()); \
    } else { \
      auto duration_as_##name = std::chrono::duration_cast<std::chrono::name>(duration); \
      return lavi::lang::api::to_object(interpreter, (int)duration_as_##name.count()); \
    } \
  });

void create_high_resolution_time_class() {
  auto high_resolution_duration_class = lavi::lang::klass::create_builtin("HighResolutionTime::Duration");
  auto high_resolution_time_class = lavi::lang::klass::create_builtin("HighResolutionTime");

  high_resolution_time_class->functions["now"] = std::make_shared<lavi::lang::function>("now", [high_resolution_time_class](lavi::lang::interpreter* interpreter) {
    auto new_object = lavi::lang::api::new_object(interpreter, high_resolution_time_class, {}, {});
    new_object->set_native(std::chrono::high_resolution_clock::now());
    return new_object;
  });

  high_resolution_time_class->instance_functions["-"] = std::make_shared<lavi::lang::function>("-", std::initializer_list<std::string>{"other"}, [high_resolution_time_class, high_resolution_duration_class](lavi::lang::interpreter* interpreter) {
    auto other_object = interpreter->current_context->positional_params[0];

    auto duration_object = lavi::lang::api::new_object(interpreter, high_resolution_duration_class, {}, {});

    auto left = interpreter->current_context->self->as<std::chrono::high_resolution_clock::time_point>();
    auto right = other_object->as<std::chrono::high_resolution_clock::time_point>();

    auto left_minus_right = left - right;

    duration_object->set_native(left_minus_right);

    return duration_object;
  });

  high_resolution_time_class->instance_functions[">"] = std::make_shared<lavi::lang::function>(">", std::initializer_list<std::string>{"other"}, [high_resolution_time_class](lavi::lang::interpreter* interpreter) {
    auto other_object = interpreter->current_context->positional_params[0];
    auto left = interpreter->current_context->self->as<std::chrono::high_resolution_clock::time_point>();
    auto right = other_object->as<std::chrono::high_resolution_clock::time_point>();
    return lavi::lang::api::to_object(interpreter, left > right);
  });

  ADD_DURATION_CAST_FUNCTION(seconds);
  ADD_DURATION_CAST_FUNCTION(milliseconds);
  ADD_DURATION_CAST_FUNCTION(microseconds);
  ADD_DURATION_CAST_FUNCTION(nanoseconds);
}