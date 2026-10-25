#include <lavi/lang/extension.hpp>

#include <iostream>
#include <string>

#include <andy.hpp>

#include <lavi/lang/interpreter.hpp>

#ifdef __linux__
#   include <dlfcn.h>
#elif defined(_WIN32)
#   include <Windows.h>
#endif

std::map<std::string_view, std::shared_ptr<lavi::lang::extension>> lavi::lang::extension::builtins;
std::map<std::string_view, std::shared_ptr<lavi::lang::extension>> symbol_to_extension;

lavi::lang::extension::extension(const std::string &name)
    : m_name(name)
{
    
}

void lavi::lang::extension::add_builtin(std::shared_ptr<lavi::lang::extension> extension)
{
    auto symbols = extension->define();

    for(const auto& symbol : symbols) {
        symbol_to_extension[symbol] = extension;
    }

    builtins.insert({ extension->name(), extension });
}

lavi::lang::extension* find_builtin(std::map<std::string_view, std::shared_ptr<lavi::lang::extension>>& builtins, std::string_view module)
{
    auto it = builtins.find(module);
    if(it != builtins.end()) {
        return it->second.get();
    }
    return nullptr;
}

// The path of a non builtin extension: <directory>/build/liblavi-<module>.so (lavi-<module>.dll on Windows),
// where directory is the one where lavi was invoked.
std::filesystem::path find_module_path(std::string_view module, const std::filesystem::path& directory)
{
    std::string library_name = std::string(module);

#if defined(__linux__)
    library_name = "liblavi-" + library_name + ".so";
#elif defined(_WIN32)
    library_name = "lavi-" + library_name + ".dll";
#else
    throw std::runtime_error("unsupported platform");
#endif

    return directory / "build" / library_name;
}

bool lavi::lang::extension::exists(std::filesystem::path current_dir, std::string_view module)
{
    auto it = builtins.find(module);
    if(it != builtins.end()) {
        return true;
    }

    std::filesystem::path module_path = find_module_path(module, current_dir);

    if(!std::filesystem::exists(module_path)) {
        return false;
    }

    return true;
}

void lavi::lang::extension::import(lavi::lang::interpreter* interpreter, std::string_view module)
{
    lavi::lang::extension* builtin = find_builtin(builtins, module);

    interpreter->stack.push_back(interpreter->global_context);
    interpreter->update_current_context();

    if(builtin) {
        interpreter->load_extension(builtin);
        interpreter->stack.pop_back();
        interpreter->update_current_context();
        return;
    }

    std::filesystem::path module_path = find_module_path(module, std::filesystem::current_path());

    if(!std::filesystem::exists(module_path)) {
        throw std::runtime_error("Module " + std::string(module) + " not found. Expect it to be builtin or to be located at " + module_path.string());
    }

    std::string module_path_str = module_path.string();
    const char* module_path_c_str = module_path_str.c_str();

    lavi::lang::extension* (*create_extension)();

#ifdef __linux__
    void* handle = dlopen(module_path_c_str, RTLD_LAZY | RTLD_GLOBAL);

    if(!handle) {
        throw std::runtime_error(dlerror());
    }

    create_extension = (lavi::lang::extension*(*)())dlsym(handle, "create_extension");

    if(!create_extension) {
        throw std::runtime_error(dlerror());
    }
#elif defined(_WIN32)
    HMODULE handle = LoadLibrary(module_path_c_str);

    if(!handle) {
        throw std::runtime_error("Failed to load library");
    }

    create_extension = (lavi::lang::extension*(*)())GetProcAddress(handle, "create_extension");

    if(!create_extension) {
        throw std::runtime_error("Failed to load symbol");
    }
#else
    throw std::runtime_error("unsupported platform");
#endif

    lavi::lang::extension* extension = create_extension();

    interpreter->load_extension(extension);

    interpreter->stack.pop_back();
    interpreter->update_current_context();
}

std::shared_ptr<lavi::lang::extension> lavi::lang::extension::which_defines(std::string_view symbol)
{
    auto it = symbol_to_extension.find(symbol);

    if(it != symbol_to_extension.end()) {
        return it->second;
    }

    return nullptr;
}