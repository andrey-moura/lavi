#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/extension.hpp>
#include <lavi/lang/config.hpp>

#include <filesystem>

std::shared_ptr<lavi::lang::klass> create_andy_config_class()
{
    lavi::lang::andy_config_class = lavi::lang::klass::create_builtin("AndyConfig");

    lavi::lang::andy_config_class->variables["src_dir"]  = lavi::lang::object::create(interpreter, lavi::lang::path_class, std::move(lavi::lang::config::src_dir()));
    lavi::lang::andy_config_class->variables["version"]  = lavi::lang::object::create(interpreter, lavi::lang::string_class, std::string(lavi::lang::config::version));
    lavi::lang::andy_config_class->variables["build"]    = lavi::lang::object::create(interpreter, lavi::lang::string_class, std::string(lavi::lang::config::build));
    lavi::lang::andy_config_class->variables["cpp"]      = lavi::lang::object::create(interpreter, lavi::lang::string_class, std::string(lavi::lang::config::cpp));
    lavi::lang::andy_config_class->variables["compiler"] = lavi::lang::object::create(interpreter, lavi::lang::string_class, std::string(lavi::lang::config::compiler));
    return lavi::lang::andy_config_class;
}