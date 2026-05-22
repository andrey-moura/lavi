#include <filesystem>

#include <andy/file.hpp>

#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>

std::shared_ptr<lavi::lang::structure> create_file_class(lavi::lang::interpreter* interpreter)
{
    auto FileClass = std::make_shared<lavi::lang::structure>("File");

        FileClass->functions["exists?"] = std::make_shared<lavi::lang::function>("exists?", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
            std::filesystem::path path;
            std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];
            if(path_object->cls == interpreter->StringClass) {
                path = path_object->as<std::string>();
            } else if(path_object->cls == interpreter->PathClass) {
                path = path_object->as<std::filesystem::path>();
            } else {
                throw std::runtime_error("invalid path");
            }
            if(std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
                return std::make_shared<lavi::lang::object>(interpreter->TrueClass);
            } else {
                return std::make_shared<lavi::lang::object>(interpreter->FalseClass);
            }
        });

        FileClass->functions["read"] = std::make_shared<lavi::lang::function>("read", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
            std::filesystem::path path;
            std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];
            if(path_object->cls == interpreter->StringClass) {
                path = path_object->as<std::string>();
            } else if(path_object->cls == interpreter->PathClass) {
                path = path_object->as<std::filesystem::path>();
            } else {
                throw std::runtime_error("invalid path");
            }
            return lavi::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(lavi::file::read_all_text<char>(path)));
        });

        FileClass->functions["read_all_lines"] = std::make_shared<lavi::lang::function>("read_all_lines", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& input_path = interpreter->current_context->positional_params[0]->as<std::string>();
            std::filesystem::path path = std::filesystem::absolute(input_path);

            if(!std::filesystem::exists(path)) {
                throw std::runtime_error("file '" + path.string() + "' does not exist");
            }

            std::vector<std::string> file = lavi::file::read_all_lines<char>(path);

            std::vector<std::shared_ptr<lavi::lang::object>> lines;

            for(auto& line : file) {
                lines.push_back(lavi::lang::object::instantiate(interpreter, interpreter->StringClass, std::move(line)));
            }

            return lavi::lang::object::instantiate(interpreter, interpreter->ArrayClass, std::move(lines));
        });

    
    return FileClass;
}