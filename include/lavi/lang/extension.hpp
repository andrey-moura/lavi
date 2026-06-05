#pragma once

#include <string>
#include <vector>

#include <andy.hpp>
#include <lavi/lang/parser.hpp>

#ifdef _WIN32
    #define ANDY_EXTENSION(name) \
    extern "C" __declspec(dllexport) lavi::lang::extension* create_extension()\
    {\
        return new name ();\
    }
#else
    #define ANDY_EXTENSION(name) \
    extern "C" lavi::lang::extension* create_extension()\
    {\
        return new name ();\
    }
#endif

namespace lavi {
    namespace lang {
        class interpreter;
        class extension : public std::enable_shared_from_this<extension>
        {
        protected:
            ANDY_EXPORT_SYMBOL extension(const std::string& name);
        public:
            static void add_builtin(std::shared_ptr<lavi::lang::extension> extension);
            static void import(lavi::lang::interpreter* interpreter, std::string_view module);
            static bool exists(std::filesystem::path current_dir, std::string_view module);
            static std::shared_ptr<lavi::lang::extension> which_defines(std::string_view symbol);
        public:
            const std::string& name() const { return m_name; }
        protected:
            static std::map<std::string_view, std::shared_ptr<lavi::lang::extension>> builtins;
        public:
            /// @brief Load the extension in the interpreter. Good for loading your custom classes. Called BEFORE the source code is executed.
            virtual void load(lavi::lang::interpreter* interpreter) { }
            /// @brief Tells the interpreter the symbols this extensions defines. This is optional and
            /// the interpreter only uses this list to suggest the user to include your extension.
            /// Ex: function 'NET::HTTP' is not defined. Did you forget to import 'net'?
            /// @param definitions The list of your extension's symbols
            virtual std::vector<std::string_view> define() { return std::vector<std::string_view>(); }
        private:
            std::string m_name;
        };
    }
};