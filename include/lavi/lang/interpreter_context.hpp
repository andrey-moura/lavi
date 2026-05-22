#pragma once

#include <deque>
#include "lavi/lang/function.hpp"

namespace lavi
{
    namespace lang
    {
        class structure;
        class object;
        class parser;
        // The context of the interpreter execution.
        struct interpreter_context
        {
            // Do NOT use shared_ptr here to avoid circular references.
            lavi::lang::object* self;
            std::shared_ptr<lavi::lang::structure> cls;
            std::map<std::string_view, std::shared_ptr<lavi::lang::object>> variables;
            std::map<std::string_view, std::shared_ptr<lavi::lang::function>> functions;
            std::map<std::string_view, std::shared_ptr<lavi::lang::inline_function>> inline_functions;
            std::vector<std::shared_ptr<lavi::lang::object>> positional_params;
            std::map<std::string_view, std::shared_ptr<lavi::lang::object>> named_params;
            std::map<std::string_view, std::shared_ptr<lavi::lang::structure>> classes;
            std::deque<std::string> string_holder;
            
            const lavi::lang::parser::ast_node* given_block = nullptr;

            bool has_returned = false;
            bool catching_exception = false;
            std::shared_ptr<lavi::lang::object> return_value;
            bool is_block_context = false;
            std::shared_ptr<lavi::lang::interpreter_context> lexical_parent = nullptr;
            std::shared_ptr<lavi::lang::interpreter_context> given_block_lexical_context = nullptr;
            const lavi::lang::parser::ast_node* caller_node = nullptr;
        };
    }; // namespace lang
}; // namespace lavi
