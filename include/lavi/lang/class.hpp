#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <lavi/lang/function.hpp>
#include <lavi/lang/interpreter_context.hpp>

namespace lavi {
    namespace lang {
        class object;
        class interpreter;
        class klass : public std::enable_shared_from_this<klass>, public scope
        {
        public:
            //for user code, use create
            klass(std::string_view __name, std::vector<lavi::lang::function> __methods = {});
            ~klass();
        public:
            std::string name;
            std::shared_ptr<lavi::lang::klass> base;
            std::vector<std::shared_ptr<lavi::lang::klass>> deriveds;
            bool is_defined = false;

            std::map<std::string, lavi::lang::parser::ast_node, std::less<>> instance_variables;
            std::map<std::string, std::shared_ptr<lavi::lang::function>, std::less<>> instance_functions;
            std::map<std::string, std::shared_ptr<lavi::lang::inline_function>, std::less<>> instance_inline_functions;
        public:
            static std::shared_ptr<lavi::lang::klass> create(std::string_view name);
            static std::shared_ptr<lavi::lang::klass> create_builtin(std::string_view name);
            static void create_builtin_classes();
        };
    };
};