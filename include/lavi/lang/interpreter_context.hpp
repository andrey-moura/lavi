#pragma once

#include <deque>
#include "lavi/lang/function.hpp"
#include "lavi/lang/scope.hpp"

namespace lavi
{
    namespace lang
    {
        class structure;
        class object;
        class parser;
        // The context of the interpreter execution.
        struct interpreter_context : public lavi::lang::scope
        {
            std::shared_ptr<lavi::lang::object> self;
            std::shared_ptr<lavi::lang::structure> cls;

            std::map<std::string_view, std::shared_ptr<lavi::lang::object>> named_params;
            std::vector<std::shared_ptr<lavi::lang::object>> positional_params;
            
            const lavi::lang::parser::ast_node* given_block = nullptr;

            bool has_returned = false;
            bool catching_exception = false;
            std::shared_ptr<lavi::lang::object> return_value;
            bool is_block_context = false;
            std::shared_ptr<lavi::lang::interpreter_context> lexical_parent = nullptr;
            std::shared_ptr<lavi::lang::interpreter_context> given_block_lexical_context = nullptr;
            const lavi::lang::parser::ast_node* caller_node = nullptr;

            std::vector<std::shared_ptr<lavi::lang::structure>> forward_declarations;
        };
    }; // namespace lang
}; // namespace lavi
