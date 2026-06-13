#include <lavi/lang/lang.hpp>
#include <lavi/lang/classes.hpp>
#include <lavi/lang/extension.hpp>
#include <lavi/lang/api.hpp>

void create_system_class()
{
    lavi::lang::system_class = lavi::lang::klass::create_builtin("System");
    
    lavi::lang::system_class->functions["OS"] = std::make_shared<lavi::lang::function>("OS", [](lavi::lang::interpreter* interpreter) {
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
        return lavi::lang::api::to_object(interpreter, std::move(std::string(current_os_name)));
    });

    lavi::lang::system_class->functions["windows?"]     = std::make_shared<lavi::lang::function>("windows?", [](lavi::lang::interpreter* interpreter) {
#ifdef _WIN32
        return lavi::lang::api::to_object(interpreter, true);
#else
        return lavi::lang::api::to_object(interpreter, false);
#endif
    });

    lavi::lang::system_class->functions["linux?"]       = std::make_shared<lavi::lang::function>("linux?", [](lavi::lang::interpreter* interpreter) {
#ifdef __linux__
        return lavi::lang::api::to_object(interpreter, true);
#else
    return lavi::lang::api::to_object(interpreter, false);
#endif
    });

    lavi::lang::system_class->functions["web_assembly?"] = std::make_shared<lavi::lang::function>("web_assembly?", [](lavi::lang::interpreter* interpreter) {
#ifdef __wasm__
        return lavi::lang::api::to_object(interpreter, true);
#else
    return lavi::lang::api::to_object(interpreter, false);
#endif
    });
}