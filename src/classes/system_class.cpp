#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/extension.hpp>
#include <lavi/lang/api.hpp>

std::shared_ptr<lavi::lang::structure> create_system_class(lavi::lang::interpreter* interpreter)
{
    auto SystemClass = std::make_shared<lavi::lang::structure>("System");
    std::string_view current_os_name;

    #ifdef _WIN32
        current_os_name = "Windows";
    #elif __linux__
        current_os_name = "Linux";
    #elif __wasm__
        current_os_name = "WebAssembly";
    #else
        throw std::runtime_error("unsupported OS");
    #endif

    SystemClass->variables["OS"] = lavi::lang::api::to_object(interpreter, std::move(std::string(current_os_name)));

    SystemClass->variables["windows?"]     = lavi::lang::api::to_object(interpreter, false);
    SystemClass->variables["linux?"]       = lavi::lang::api::to_object(interpreter, false);
    SystemClass->variables["web_assembly?"] = lavi::lang::api::to_object(interpreter, false);

#ifdef _WIN32
    SystemClass->variables["windows?"] = lavi::lang::api::to_object(interpreter, true);
#elif __linux__
    SystemClass->variables["linux?"] = lavi::lang::api::to_object(interpreter, true);
#elif __wasm__
    SystemClass->variables["web_assembly?"] = lavi::lang::api::to_object(interpreter, true);
#endif

    return SystemClass;
}