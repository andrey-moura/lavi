#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/extension.hpp>
#include <lavi/lang/config.hpp>
#include <lavi/lang/api.hpp>

#include <filesystem>

std::shared_ptr<lavi::lang::klass> create_andy_config_class()
{
    lavi::lang::andy_config_class = lavi::lang::klass::create_builtin("AndyConfig");

    lavi::lang::andy_config_class->functions["src_dir"]  = std::make_shared<lavi::lang::function>("src_dir", [](lavi::lang::interpreter* interpreter) {
        return lavi::lang::api::to_object(interpreter, std::move(lavi::lang::config::src_dir()));
    });

    lavi::lang::andy_config_class->functions["version"]  = std::make_shared<lavi::lang::function>("version", [](lavi::lang::interpreter* interpreter) {
        return lavi::lang::api::to_object(interpreter, std::string(lavi::lang::config::version));
    });

    lavi::lang::andy_config_class->functions["build"]    = std::make_shared<lavi::lang::function>("build", [](lavi::lang::interpreter* interpreter) {
        return lavi::lang::api::to_object(interpreter, std::string(lavi::lang::config::build));
    });

    lavi::lang::andy_config_class->functions["cpp"]      = std::make_shared<lavi::lang::function>("cpp", [](lavi::lang::interpreter* interpreter) {
        return lavi::lang::api::to_object(interpreter, std::string(lavi::lang::config::cpp));
    });

    lavi::lang::andy_config_class->functions["compiler"] = std::make_shared<lavi::lang::function>("compiler", [](lavi::lang::interpreter* interpreter) {
        return lavi::lang::api::to_object(interpreter, std::string(lavi::lang::config::compiler));
    });

    return lavi::lang::andy_config_class;
}