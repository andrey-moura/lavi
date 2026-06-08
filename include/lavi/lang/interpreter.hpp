#pragma once

#include <vector>
#include <memory>

#include <lavi/lang/parser.hpp>
#include <lavi/lang/class.hpp>
#include <lavi/lang/function.hpp>
#include <lavi/lang/object.hpp>
#include <lavi/lang/interpreter_context.hpp>

namespace lavi
{
    namespace lang
    {
        class extension;
        // This class is responsible of storing all resources needed by an andylang program.
        // It will store all classes, objects, methods, variables, call stack, etc.
        class interpreter
        {
        public:
            /// @brief Construct a new interpreter object. When the interpreter object is constructed, it will
            // initialize all resources needed to run the interpreter. If you want to declare the interpreter
            // but not initialize it, you can use a interpreter pointer and initialize it later.
            interpreter();
            ~interpreter() = default;
        public:
            std::filesystem::path input_file_path;
            lavi::lang::lexer* main_lexer = nullptr;
        public:
            /// @brief Load a class into the vm. The class is kept alive by the vm untill it is destroyed.
            /// @param cls The class to be loaded. It is kept alive by the vm untill it is destroyed. It is globally accessible.
            void load(std::shared_ptr<lavi::lang::structure> cls);

            /// @brief Exeuctes a syntax tree into the interpreter. Note that if the code has while loops with no exit condition, this method will never return.
            /// @param cls The syntax tree to exeuctes. All its childs (not recursively) will be executed.
            std::shared_ptr<lavi::lang::object> execute(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_all(
                std::vector<lavi::lang::parser::ast_node>::const_iterator begin,
                std::vector<lavi::lang::parser::ast_node>::const_iterator end
            );
            std::shared_ptr<lavi::lang::object> execute_all(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_context(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_classdecl(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_classdecl_base(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_fn_decl(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_fn_return(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_fn_call(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_fn_params(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_fn_object(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_valuedecl(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_pair(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_interpolated_string(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_arraydecl(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_hashdecl(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_vardecl(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_decltype(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_declname(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_declstatic(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_conditional(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_while(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_for(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_for_start(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_for_step(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_for_end(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_break(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_else(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_condition(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_yield(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_try(const lavi::lang::parser::ast_node& source_code);
            std::shared_ptr<lavi::lang::object> execute_throw(const lavi::lang::parser::ast_node& source_code);

            std::shared_ptr<lavi::lang::structure> find_class(const std::string_view& name);

            const std::shared_ptr<lavi::lang::object> node_to_object(const lavi::lang::parser::ast_node& node, std::shared_ptr<lavi::lang::structure> cls = nullptr, std::shared_ptr<lavi::lang::object> object = nullptr);

            void load_extension(lavi::lang::extension* extension);

            std::shared_ptr<lavi::lang::interpreter_context> current_context = nullptr;
            std::shared_ptr<lavi::lang::interpreter_context> previous_context = nullptr;
            std::shared_ptr<lavi::lang::interpreter_context> global_context = nullptr;

            /// @brief The call stack.
            std::vector<std::shared_ptr<interpreter_context>> stack;
            std::vector<std::string_view> args;
        protected:
            std::vector<lavi::lang::extension*> extensions;

            bool is_global_context() const
            {
                return stack.size() == 1;
            }
            public:

            void update_current_context();
            void push_context();
            void push_block_context();
            void push_context(std::shared_ptr<lavi::lang::object> object);
            void pop_context();
            void set_current_context_object(std::shared_ptr<lavi::lang::object> object);
            void push_context_with_object(std::shared_ptr<lavi::lang::object> object);
        protected:
            /// @brief Initialize the interpreter. This method will create the global classes and objects. It also load extensions.
            void init();
        };
    }  
}; // namespace lavi