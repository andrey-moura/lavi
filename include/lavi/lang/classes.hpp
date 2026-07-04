#pragma once

#include <memory>

#include <lavi/lang/class.hpp>

namespace lavi
{
  namespace lang
  {
    /// @brief This variable is used to check if the classes have been created.
    extern bool classes_created;
    /// @brief All the built in classes.
    extern std::vector<std::shared_ptr<lavi::lang::klass>> builtin_classes;

    /// @brief The global false class.
    extern std::shared_ptr<lavi::lang::klass> std_class;

    /// @brief The global false class.
    extern std::shared_ptr<lavi::lang::klass> false_class;
    /// @brief The global true class.
    extern std::shared_ptr<lavi::lang::klass> true_class;

    /// @brief The global string class.
    extern std::shared_ptr<lavi::lang::klass> string_class;

    /// @brief The global integer class.
    extern std::shared_ptr<lavi::lang::klass> integer_class;

    /// @brief The global double class.
    extern std::shared_ptr<lavi::lang::klass> double_class;

    /// @brief The global float class.
    extern std::shared_ptr<lavi::lang::klass> float_class;

    /// @brief The global file class.
    extern std::shared_ptr<lavi::lang::klass> file_class;

    /// @brief The global array class.
    extern std::shared_ptr<lavi::lang::klass> array_class;

    /// @brief The global null class.
    extern std::shared_ptr<lavi::lang::klass> null_class;

    /// @brief The global hash class.
    extern std::shared_ptr<lavi::lang::klass> hash_class;

    /// @brief The global system class.
    extern std::shared_ptr<lavi::lang::klass> system_class;

    /// @brief The global path class.
    extern std::shared_ptr<lavi::lang::klass> path_class;

    /// @brief The global andy config class.
    extern std::shared_ptr<lavi::lang::klass> andy_config_class;

    /// @brief The global class class.
    extern std::shared_ptr<lavi::lang::klass> class_class;

    /// @brief The global function class.
    extern std::shared_ptr<lavi::lang::klass> function_class;

    /// @brief The global exception class.
    extern std::shared_ptr<lavi::lang::klass> exception_class;

    /// @brief The global no function error class.
    extern std::shared_ptr<lavi::lang::klass> no_function_error_class;

    /// @brief The global undefined class error class.
    extern std::shared_ptr<lavi::lang::klass> undefined_class_error_class;

    /// @brief The global runtime error class.
    extern std::shared_ptr<lavi::lang::klass> runtime_error_class;
  };
}