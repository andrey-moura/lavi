#include <filesystem>
#include <list>

#include <andy/file.hpp>
#include <andy/lang/api.hpp>
#include <andy/lang/lang.hpp>
#include <andy/lang/interpreter.hpp>

std::shared_ptr<andy::lang::structure> create_directory_class(andy::lang::interpreter* interpreter)
{
    auto DirectoryClass = std::make_shared<andy::lang::structure>("Dir");

    DirectoryClass->instance_functions["init"] = std::make_shared<andy::lang::function>("init", std::initializer_list<std::string>{"path"}, [](andy::lang::interpreter* interpreter) {
        auto object = interpreter->current_context->self;
        object->set_native<std::filesystem::path>(std::move(std::filesystem::path(interpreter->current_context->positional_params[0]->as<std::string>())));

        return nullptr;
    });

    DirectoryClass->instance_functions["glob"] = std::make_shared<andy::lang::function>("glob", std::initializer_list<std::string>{"pattern"}, [](andy::lang::interpreter* interpreter) {
        std::filesystem::path& root_path = interpreter->current_context->self->as<std::filesystem::path>();
#ifdef _WIN32
        std::string pattern = interpreter->current_context->positional_params[0]->as<std::string>();
        std::replace(pattern.begin(), pattern.end(), '/', '\\');
#else
        const std::string& pattern = interpreter->current_context->positional_params[0]->as<std::string>();
#endif

        std::vector<std::shared_ptr<andy::lang::object>> results;

        std::string_view pattern_view(pattern);

        std::vector<std::string_view> parts;
        parts.reserve(pattern_view.size() / 8); // average file name length is 8, so this is a good estimation

        while(pattern_view.size()) {
            size_t pos = pattern_view.find('/');

            if(pos == std::string_view::npos) {
                parts.push_back(pattern_view.substr(0, pattern_view.size()));
                break;
            } else {
                parts.push_back(pattern_view.substr(0, pos));
            }

            pattern_view.remove_prefix(pos + 1);
        }

        std::function<void(std::filesystem::path)> recurse;
        int current_level = -1;
        
        recurse = [&](std::filesystem::path current_path) {
            current_level++;
            std::string_view current_matching_part = parts[current_level];
            bool should_enter_any_directory = false;
            bool shoud_start_matching_files = false;
            if(current_level == parts.size() - 1)
            {
                shoud_start_matching_files = true;

                if(current_matching_part.starts_with('*')) {
                    current_matching_part.remove_prefix(1);
                }
            }
            if(current_matching_part == "**") {
                should_enter_any_directory = true;
            }

            for(auto& entry : std::filesystem::directory_iterator(current_path)) {
                const std::filesystem::path& entry_path = entry.path();
                std::filesystem::path relative_path = std::filesystem::relative(entry_path, current_path);
                if(entry.is_directory()) {
                    bool should_enter_directory = false;
                    if(!should_enter_any_directory) {
                        if(relative_path == current_matching_part) {
                            should_enter_directory = true;
                        }
                    }

                    if(should_enter_any_directory) {
                        should_enter_directory = true;
                    }
                    if(should_enter_directory) {
                        recurse(entry_path);
                    }

                } else if(shoud_start_matching_files && entry.is_regular_file()) {
                    if(relative_path.filename().string().ends_with(current_matching_part)) {
                        results.push_back(andy::lang::object::instantiate(interpreter, interpreter->PathClass, std::move(entry_path)));
                    }
                }
            }

            should_enter_any_directory = false;
            shoud_start_matching_files = false;
            current_level--;
        };

        recurse(root_path);

        return andy::lang::object::instantiate(interpreter, interpreter->ArrayClass, std::move(results));
    });

    DirectoryClass->functions["exists?"] = std::make_shared<andy::lang::function>("exists?", std::initializer_list<std::string>{"path"}, [](andy::lang::interpreter* interpreter) {
        std::filesystem::path path;
        std::shared_ptr<andy::lang::object> path_object = interpreter->current_context->positional_params[0];
        if(path_object->cls == interpreter->StringClass) {
            path = path_object->as<std::string>();
        } else if(path_object->cls == interpreter->PathClass) {
            path = path_object->as<std::filesystem::path>();
        } else {
            throw std::runtime_error("invalid path");
        }
        if(std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            return std::make_shared<andy::lang::object>(interpreter->TrueClass);
        } else {
            return std::make_shared<andy::lang::object>(interpreter->FalseClass);
        }
    });

    DirectoryClass->functions["create"] = std::make_shared<andy::lang::function>("create", std::initializer_list<std::string>{"path"}, [](andy::lang::interpreter* interpreter) {
        std::filesystem::path path;
        std::shared_ptr<andy::lang::object> path_object = interpreter->current_context->positional_params[0];
        if(path_object->cls == interpreter->StringClass) {
            path = path_object->as<std::string>();
        } else if(path_object->cls == interpreter->PathClass) {
            path = path_object->as<std::filesystem::path>();
        } else {
            throw std::runtime_error("invalid path");
        }
        std::filesystem::create_directory(path);
        return nullptr;
    });

    DirectoryClass->functions["home"] = std::make_shared<andy::lang::function>("home", [](andy::lang::interpreter* interpreter) {
        std::filesystem::path path = std::filesystem::path(std::getenv("HOME"));
        if(path.empty()) {
            throw std::runtime_error("Unable to retrieve home directory");
        }
        return andy::lang::object::instantiate(interpreter, interpreter->PathClass, std::move(path));
    });

    DirectoryClass->functions["current"] = std::make_shared<andy::lang::function>("current", [DirectoryClass](andy::lang::interpreter* interpreter) {
        std::filesystem::path path = std::filesystem::current_path();
        if(path.empty()) {
            throw std::runtime_error("Unable to retrieve current directory");
        }
        return andy::lang::object::instantiate(interpreter, DirectoryClass, std::move(path));
    });

    return DirectoryClass;
}