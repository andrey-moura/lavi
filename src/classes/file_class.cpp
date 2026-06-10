#include <filesystem>
#include <fstream>

#include <lavi/lang/lang.hpp>
#include <lavi/lang/api.hpp>
#include <lavi/lang/interpreter.hpp>

std::shared_ptr<lavi::lang::klass> create_file_class()
{
    lavi::lang::file_class = lavi::lang::klass::create_builtin("File");

        lavi::lang::file_class->functions["exists?"] = std::make_shared<lavi::lang::function>("exists?", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
            std::filesystem::path path;
            std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];
            if(path_object->klass == lavi::lang::string_class) {
                path = path_object->as<std::string>();
            } else if(path_object->klass == lavi::lang::path_class) {
                path = path_object->as<std::filesystem::path>();
            } else {
                throw std::runtime_error("invalid path");
            }
            if(std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
                return std::make_shared<lavi::lang::object>(lavi::lang::true_class);
            } else {
                return std::make_shared<lavi::lang::object>(lavi::lang::false_class);
            }
        });

        lavi::lang::file_class->functions["read"] = std::make_shared<lavi::lang::function>("read", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
            std::filesystem::path path;
            std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];
            if(path_object->klass == lavi::lang::string_class) {
                path = path_object->as<std::string>();
            } else if(path_object->klass == lavi::lang::path_class) {
                path = path_object->as<std::filesystem::path>();
            } else {
                throw std::runtime_error("invalid path");
            }

            std::string content;
            std::ifstream stream(path, std::ios::in);

            stream.seekg(0, std::ios::end);
            content.resize(stream.tellg());
            stream.seekg(0, std::ios::beg);
            stream.read(&content[0], content.size());

            return lavi::lang::api::to_object(interpreter, content);
        });

        lavi::lang::file_class->functions["read_all_lines"] = std::make_shared<lavi::lang::function>("read_all_lines", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
            const std::string& input_path = interpreter->current_context->positional_params[0]->as<std::string>();
            std::filesystem::path path = std::filesystem::absolute(input_path);

            if(!std::filesystem::exists(path)) {
                throw std::runtime_error("file '" + path.string() + "' does not exist");
            }

            std::vector<std::shared_ptr<lavi::lang::object>> lines;

            std::ifstream stream(path, std::ios::in);

            std::string text;

            if(stream.is_open()) {
                while(std::getline(stream, text)) {
                    lines.push_back(lavi::lang::api::to_object(interpreter, text));
                }
            }

            return lavi::lang::api::to_object(interpreter, lines);
        });

    
    return lavi::lang::file_class;
}