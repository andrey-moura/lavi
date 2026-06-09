#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include <lavi/lang/parser.hpp>

namespace lavi {
    namespace lang {
        class object;
        class klass;
        class function;
        class interpreter;
        using inline_function = std::shared_ptr<lavi::lang::object>(*)(lavi::lang::interpreter*, std::shared_ptr<lavi::lang::object>&, const lavi::lang::parser::ast_node&);
        enum class function_storage_type {
            instance_function,
            class_function,
        };
        struct fn_parameter
        {
            fn_parameter() = default;
            fn_parameter(std::string_view __name);
            fn_parameter(std::string __name, bool __named, const lavi::lang::parser::ast_node* __default_value_node)
                : name(std::move(__name)), named(__named), default_value_node(__default_value_node) {
            }
            std::string name;
            const lavi::lang::parser::ast_node* default_value_node = nullptr;
            bool named = false;
        };
        class function
        {
        public:
            std::string_view name;
            lavi::lang::parser::ast_node block_ast;
            function_storage_type storage_type;
            std::vector<fn_parameter> positional_params;
            std::vector<fn_parameter> named_params;
            std::function<std::shared_ptr<lavi::lang::object>(lavi::lang::interpreter*)> native_function;

            function() = default;

            function(std::string_view __name, std::vector<std::string> __params, lavi::lang::parser::ast_node __block)
                : name(__name), block_ast(std::move(__block)) {
                init_params(__params);
            };

            function(std::string_view __name, std::vector<fn_parameter> __params, lavi::lang::parser::ast_node __block)
                : name(__name), block_ast(std::move(__block)) {
                for(auto& param : __params) {
                    if(param.named) {
                        named_params.push_back(std::move(param));
                    } else {
                        positional_params.push_back(std::move(param));
                    }
                }
            };

            function(std::string_view name, std::vector<std::string> __params, std::function<std::shared_ptr<lavi::lang::object>(lavi::lang::interpreter*)> fn)
                : name(name), native_function(fn) {
                init_params(__params);
            }

            function(std::string_view name, std::function<std::shared_ptr<lavi::lang::object>(lavi::lang::interpreter*)> fn)
                : name(name), native_function(fn) {
            }

            protected:
                void init_params(std::vector<std::string> __params);
        };
    }
}