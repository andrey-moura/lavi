#pragma once

#include <memory>

#include <lavi/lang/class.hpp>

namespace lavi
{
  namespace lang
  {
    /// @brief The global false class.
    std::shared_ptr<lavi::lang::structure> FalseClass;
    /// @brief The global true class.
    std::shared_ptr<lavi::lang::structure> TrueClass;

    /// @brief The global string class.
    std::shared_ptr<lavi::lang::structure> StringClass;

    /// @brief The global integer class.
    std::shared_ptr<lavi::lang::structure> IntegerClass;

    /// @brief The global double class.
    std::shared_ptr<lavi::lang::structure> DoubleClass;

    /// @brief The global float class.
    std::shared_ptr<lavi::lang::structure> FloatClass;

    /// @brief The global file class.
    std::shared_ptr<lavi::lang::structure> FileClass;

    /// @brief The global array class.
    std::shared_ptr<lavi::lang::structure> ArrayClass;

    /// @brief The global null class.
    std::shared_ptr<lavi::lang::structure> NullClass;

    /// @brief The global hash class.
    std::shared_ptr<lavi::lang::structure> HashClass;

    /// @brief The global system class.
    std::shared_ptr<lavi::lang::structure> SystemClass;

    /// @brief The global path class.
    std::shared_ptr<lavi::lang::structure> PathClass;

    /// @brief The global andy config class.
    std::shared_ptr<lavi::lang::structure> AndyConfigClass;

    /// @brief The global class class.
    std::shared_ptr<lavi::lang::structure> ClassClass;

    /// @brief The global function class.
    std::shared_ptr<lavi::lang::structure> FunctionClass;

    /// @brief The global exception class.
    std::shared_ptr<lavi::lang::structure> ExceptionClass;

    /// @brief The global no function error class.
    std::shared_ptr<lavi::lang::structure> NoFunctionErrorClass;

    /// @brief The global runtime error class.
    std::shared_ptr<lavi::lang::structure> RuntimeErrorClass;
  };
}