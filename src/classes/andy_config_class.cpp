#include <andy/lang/lang.hpp>
#include <andy/lang/interpreter.hpp>
#include <andy/lang/extension.hpp>
#include <andy/lang/config.hpp>

#include <filesystem>

std::shared_ptr<lavi::lang::structure> create_andy_config_class(lavi::lang::interpreter* interpreter)
{
    auto AndyConfigClass = std::make_shared<lavi::lang::structure>("AndyConfig");

    AndyConfigClass->variables["src_dir"]  = lavi::lang::object::create(interpreter, interpreter->PathClass, std::move(lavi::lang::config::src_dir()));
    AndyConfigClass->variables["version"]  = lavi::lang::object::create(interpreter, interpreter->StringClass, std::string(lavi::lang::config::version));
    AndyConfigClass->variables["build"]    = lavi::lang::object::create(interpreter, interpreter->StringClass, std::string(lavi::lang::config::build));
    AndyConfigClass->variables["cpp"]      = lavi::lang::object::create(interpreter, interpreter->StringClass, std::string(lavi::lang::config::cpp));
    AndyConfigClass->variables["compiler"] = lavi::lang::object::create(interpreter, interpreter->StringClass, std::string(lavi::lang::config::compiler));
    return AndyConfigClass;
}