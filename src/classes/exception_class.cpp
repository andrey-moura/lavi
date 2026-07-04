#include "lavi/lang/interpreter.hpp"
#include "lavi/lang/lang.hpp"
#include "lavi/lang/classes.hpp"
#include "lavi/lang/error.hpp"
#include "lavi/lang/api.hpp"

void create_no_function_error_class()
{
  lavi::lang::no_function_error_class = lavi::lang::klass::create_builtin("NoFunctionError");
  lavi::lang::no_function_error_class->base = lavi::lang::exception_class;
}

void create_runtime_error_class()
{
  lavi::lang::runtime_error_class = lavi::lang::klass::create_builtin("RuntimeError");
  lavi::lang::runtime_error_class->base = lavi::lang::exception_class;
}

void create_exception_class()
{
  lavi::lang::exception_class = lavi::lang::klass::create_builtin("Exception");

  lavi::lang::exception_class->instance_functions["init"] = std::make_shared<lavi::lang::function>("init", std::initializer_list<std::string>{"message"}, [](lavi::lang::interpreter* interpreter) {
    interpreter->current_context->self->variables["message"] = interpreter->current_context->positional_params[0];
    return nullptr;
  });

  create_no_function_error_class();
  create_runtime_error_class();
}