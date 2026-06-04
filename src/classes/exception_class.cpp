#include "lavi/lang/interpreter.hpp"
#include "lavi/lang/lang.hpp"
#include "lavi/lang/error.hpp"
#include "lavi/lang/api.hpp"

std::shared_ptr<lavi::lang::structure> create_exception_class(lavi::lang::interpreter* interpreter)
{
  std::shared_ptr<lavi::lang::structure> ExceptionClass = std::make_shared<lavi::lang::structure>("Exception");

  ExceptionClass->instance_functions["init"] = std::make_shared<lavi::lang::function>("init", std::initializer_list<std::string>{"message"}, [](lavi::lang::interpreter* interpreter) {
    interpreter->current_context->self->variables["message"] = interpreter->current_context->positional_params[0];
    return nullptr;
  });

  return ExceptionClass;
}

std::shared_ptr<lavi::lang::structure> create_no_function_error_class(lavi::lang::interpreter* interpreter)
{
  std::shared_ptr<lavi::lang::structure> NoFunctionErrorClass = std::make_shared<lavi::lang::structure>("NoFunctionError");
  NoFunctionErrorClass->base = interpreter->ExceptionClass;

  return NoFunctionErrorClass;
}

std::shared_ptr<lavi::lang::structure> create_runtime_error_class(lavi::lang::interpreter* interpreter)
{
  std::shared_ptr<lavi::lang::structure> RuntimeErrorClass = std::make_shared<lavi::lang::structure>("RuntimeError");
  RuntimeErrorClass->base = interpreter->ExceptionClass;

  return RuntimeErrorClass;
}

