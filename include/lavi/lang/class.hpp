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
            std::string_view name;
            std::shared_ptr<lavi::lang::klass> base;
            std::vector<std::shared_ptr<lavi::lang::klass>> deriveds;

            std::map<std::string_view, const lavi::lang::parser::ast_node*> instance_variables;
            std::map<std::string_view, std::shared_ptr<lavi::lang::function>> instance_functions;
            std::map<std::string_view, std::shared_ptr<lavi::lang::inline_function>> instance_inline_functions;
        public:
            static std::shared_ptr<lavi::lang::klass> create(std::string_view name);
            static std::shared_ptr<lavi::lang::klass> create_builtin(std::string_view name);
            static void create_structures();
        };
    };
};