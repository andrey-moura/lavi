#include <lavi/lang/lang.hpp>
#include <lavi/lang/classes.hpp>
#include <lavi/lang/extension.hpp>
#include <lavi/lang/api.hpp>

void create_system_class()
{
    lavi::lang::system_class = lavi::lang::klass::create_builtin("System");
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

    lavi::lang::system_class->variables["OS"] = lavi::lang::api::to_object(interpreter, std::move(std::string(current_os_name)));

    lavi::lang::system_class->variables["windows?"]     = lavi::lang::api::to_object(interpreter, false);
    lavi::lang::system_class->variables["linux?"]       = lavi::lang::api::to_object(interpreter, false);
    lavi::lang::system_class->variables["web_assembly?"] = lavi::lang::api::to_object(interpreter, false);
    lavi::lang::system_class->variables["linux?"]       = lavi::lang::api::to_object(interpreter, false);
    lavi::lang::system_class->variables["web_assembly?"] = lavi::lang::api::to_object(interpreter, false);

#ifdef _WIN32
    lavi::lang::system_class->variables["windows?"] = lavi::lang::api::to_object(interpreter, true);
#elif __linux__
    lavi::lang::system_class->variables["linux?"] = lavi::lang::api::to_object(interpreter, true);
#elif __wasm__
    lavi::lang::system_class->variables["web_assembly?"] = lavi::lang::api::to_object(interpreter, true);
#endif
}