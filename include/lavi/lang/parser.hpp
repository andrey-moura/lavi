#pragma once

#include <filesystem>
#include <map>
#include <functional>
#include <string>

#include <andy/core.hpp>

#include <lavi/lang/lexer.hpp>

namespace lavi
{
    namespace lang
    {
        class parser
        {
        public:
            parser();
            ~parser() = default;
        public:
            class exception : public std::exception
            {
            public:
                exception(
                    std::string message,
                    const lavi::lang::lexer::token& token
                );
                exception(const lavi::lang::lexer::token& token);
            protected:
                const lavi::lang::lexer::token& m_token;
                std::string m_message;
            public:
                const lavi::lang::lexer::token& token() const { return m_token; }
                const char* what() const noexcept override { return m_message.c_str(); }
            };
            class unexpected_token_error : public lavi::lang::parser::exception
            {
            public:
                unexpected_token_error(const lavi::lang::lexer::token& token, std::string_view expected = "");
            private:
                std::string_view m_expected;
            private:
                std::string generate_message() const;
            };
        public:
            enum ast_node_type {
                ast_node_undefined,

                ast_node_unit,
                ast_node_expansion,

                ast_node_context,

                ast_node_classdecl,
                ast_node_classdecl_base,

                ast_node_fn_decl,
                ast_node_fn_return,
                ast_node_fn_call,
                ast_node_fn_params,
                ast_node_fn_object,

                ast_node_valuedecl,
                ast_node_pair,
                ast_node_interpolated_string,
                ast_node_arraydecl,
                ast_node_hashdecl,
                ast_node_vardecl,

                ast_node_decltype,
                ast_node_declname,
                ast_node_declstatic,

                ast_node_conditional,
                ast_node_while,
                ast_node_for,
                ast_node_for_start,
                ast_node_for_step,
                ast_node_for_end,
                ast_node_foreach,
                ast_node_break,
                ast_node_else,
                ast_node_condition,
                ast_node_yield,
                ast_node_yield_block,
                ast_node_throw,
                ast_node_try,
                ast_node_catch,

                ast_node_enum
            };
            class ast_node
            {
            public:
                ast_node()
                    : m_type(ast_node_type::ast_node_undefined) {
                    
                }
                ast_node(ast_node_type __type)
                    : m_type(__type) {
                    
                }
                ast_node(lavi::lang::lexer::token __token, ast_node_type __type)
                    : m_token(__token), m_type(__type) {
                    
                }
            protected:
                lavi::lang::lexer::token m_token;
                lavi::lang::lexer::token m_end_token;
                ast_node_type m_type;
                std::vector<ast_node> m_children;
            public:
                bool is_undefined() const {
                    return m_type == ast_node_type::ast_node_undefined;
                }
            // Setters
            public:
                void add_child(ast_node child) {
                    m_children.push_back(std::move(child));
                }
                void set_type(ast_node_type __type)
                {
                    m_type = __type;
                }
                void set_token(lavi::lang::lexer::token __token)
                {
                    m_token = std::move(__token);
                }
                void set_end_token(lavi::lang::lexer::token __token)
                {
                    m_end_token = std::move(__token);
                }
            // Getters
            public:
                lavi::lang::lexer::token& token() {
                    return m_token;
                }

                const lavi::lang::lexer::token& token() const {
                    return m_token;
                }

                lavi::lang::lexer::token& end_token() {
                    return m_end_token;
                }

                const lavi::lang::lexer::token& end_token() const {
                    return m_end_token;
                }

                std::vector<ast_node>& childrens() {
                    return m_children;
                }

                const std::vector<ast_node>& childrens() const {
                    return m_children;
                }

                ast_node_type type() const {
                    return m_type;
                }

                const ast_node* child_from_type(const lavi::lang::parser::ast_node_type& __type) const {
                    for(auto& child : m_children) {
                        if(child.type() == __type) {
                            return &child;
                        }
                    }

                    return nullptr;
                }

                ast_node* child_from_type(const lavi::lang::parser::ast_node_type& __type) {
                    for(auto& child : m_children) {
                        if(child.type() == __type) {
                            return &child;
                        }
                    }

                    return nullptr;
                }

                const lavi::lang::lexer::token* child_token_from_type(const lavi::lang::parser::ast_node_type& __type) const {
                    return &child_from_type(__type)->token();
                }

                const std::string_view child_content_from_type(const ast_node_type& __type) const {
                    return child_token_from_type(__type)->content;
                }

                const std::string_view decname() const {
                    return child_content_from_type(ast_node_type::ast_node_declname);
                }

                const std::string_view decl_type() const {
                    return child_content_from_type(ast_node_type::ast_node_decltype);
                }

                const std::string_view value() const {
                    return child_content_from_type(ast_node_type::ast_node_valuedecl);
                }

                const ast_node* condition() const {
                    return child_from_type(ast_node_type::ast_node_condition);
                }

                const ast_node* block() const {
                    return child_from_type(ast_node_type::ast_node_context);
                }

                const ast_node* context() const {
                    return child_from_type(ast_node_type::ast_node_context);
                }

                const ast_node* fn_object() const {
                    return child_from_type(ast_node_type::ast_node_fn_object);
                }
            };
        protected:
            std::filesystem::path current_path;
        public:
            std::filesystem::path absolute(const std::string& path) {
                return current_path / path;
            }
        public:
            lavi::lang::parser::ast_node parse_node(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_all(lavi::lang::lexer& lexer);

        // Commons extract functions used by parsers
        public:
            /// @brief Extract a function call parameters. You must consume the ')' token.
            /// @param lexer The lexer.
            /// @return A function call parameters node.
            lavi::lang::parser::ast_node extract_fn_call_params(lavi::lang::lexer& lexer);
            /// @brief Extract a pair
            /// @param lexer The lexer.
            /// @return The pair node.
            lavi::lang::parser::ast_node extract_pair(lavi::lang::lexer& lexer);
            /// @brief Parse an identifier or a literal. If it's followed by a '(', it's a function call, so it will extract the parameters and return a function call node. Otherwise, it will return a declname or a valuedecl node.
            /// @param lexer The lexer.
            /// @param chain Whether to allow chaining function calls and member accesses.
            /// @param keyword Whether a keyword can be parsed as an identifier. This is useful when we can have something like obj.class
            /// @return 
            lavi::lang::parser::ast_node parse_identifier_or_literal(lavi::lang::lexer& lexer, bool chain = true, std::vector<std::string_view> keyword = {});
        // Parsers functions
        protected:
            /// @brief Parse a keyword.
            /// @param lexer The lexer.
            /// @return A keyword node.
            lavi::lang::parser::ast_node parse_keyword(lavi::lang::lexer& lexer);
            /// @brief Parse a delimiter.
            /// @param lexer The lexer.
            /// @return A delimiter node.
            lavi::lang::parser::ast_node parse_delimiter(lavi::lang::lexer& lexer);
            /// @brief Parse a context.
            /// @param lexer The lexer.
            /// @return A context node.
            lavi::lang::parser::ast_node parse_eof(lavi::lang::lexer& lexer);
            /// @brief Parse a preprocessing directive.
            /// @param lexer The lexer.
            /// @return An exception
            lavi::lang::parser::ast_node parse_preprocessor(lavi::lang::lexer& lexer);
        protected:
            lavi::lang::parser::ast_node parse_keyword_class(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_var(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_function(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_return(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_if(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_namespace(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_loop(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_for(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_foreach(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_while(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_break(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_static(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_yield(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_within(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_throw(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_try(lavi::lang::lexer& lexer);
            lavi::lang::parser::ast_node parse_keyword_enum(lavi::lang::lexer& lexer);
        };
    }
}; // namespace lavi