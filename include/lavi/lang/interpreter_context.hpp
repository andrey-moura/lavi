#pragma once

#include <deque>
#include "lavi/lang/function.hpp"
#include "lavi/lang/scope.hpp"

namespace lavi
{
    namespace lang
    {
        class klass;
        class object;
        class parser;
        // The context of the interpreter execution.
        struct interpreter_context : public lavi::lang::scope
        {
            std::shared_ptr<lavi::lang::object> self;
            std::shared_ptr<lavi::lang::klass> klass;

            std::map<std::string, std::shared_ptr<lavi::lang::object>, std::less<>> named_params;
            std::vector<std::shared_ptr<lavi::lang::object>> positional_params;
            
            const lavi::lang::parser::ast_node* given_block = nullptr;

            bool has_returned = false;
            bool catching_exception = false;
            std::shared_ptr<lavi::lang::object> return_value;
            bool is_block_context = false;
            std::shared_ptr<lavi::lang::interpreter_context> lexical_parent = nullptr;
            std::shared_ptr<lavi::lang::interpreter_context> given_block_lexical_context = nullptr;
            const lavi::lang::parser::ast_node* caller_node = nullptr;

            std::map<std::string, std::shared_ptr<lavi::lang::klass>, std::less<>> forward_declarations;

            std::string fully_qualified_name(std::string_view name) const;
        };
    }; // namespace lang
}; // namespace lavi
