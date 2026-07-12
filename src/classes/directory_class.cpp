#include <filesystem>
#include <list>

#include <andy/file.hpp>
#include <lavi/lang/api.hpp>
#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>

std::shared_ptr<lavi::lang::structure> create_directory_class(lavi::lang::interpreter* interpreter)
{
    auto DirectoryClass = std::make_shared<lavi::lang::structure>("Dir");

    DirectoryClass->instance_functions["init"] = std::make_shared<lavi::lang::function>("init", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
        auto object = interpreter->current_context->self;
        object->set_native<std::filesystem::path>(std::move(std::filesystem::path(interpreter->current_context->positional_params[0]->as<std::string>())));

        return nullptr;
    });

    DirectoryClass->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
        auto& object = interpreter->current_context->self;
        auto& path = object->as<std::filesystem::path>();

        return lavi::lang::api::to_object(interpreter, path.string());
    });

    DirectoryClass->instance_functions["path"] = std::make_shared<lavi::lang::function>("path", [](lavi::lang::interpreter* interpreter) {
        auto& object = interpreter->current_context->self;
        auto& path = object->as<std::filesystem::path>();

        return lavi::lang::api::to_object(interpreter, path);
    });

    DirectoryClass->instance_functions["glob"] = std::make_shared<lavi::lang::function>("glob", std::initializer_list<std::string>{"pattern"}, [](lavi::lang::interpreter* interpreter) {
        std::filesystem::path& root_path = interpreter->current_context->self->as<std::filesystem::path>();
        const std::string& pattern = interpreter->current_context->positional_params[0]->as<std::string>();

        std::vector<std::shared_ptr<lavi::lang::object>> results;

        std::filesystem::path current_path = root_path;

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

        std::function<void()> recurse;
        std::string_view current_part;
        int current_part_index = 0;

        auto push_part = [&]() -> void {
            current_part_index++;
            if(current_part_index < parts.size()) {
                current_part = parts[current_part_index];
            }
        };

        auto pop_part = [&]() -> void {
            current_part_index--;
            if(current_part_index >= 0) {
                current_part = parts[current_part_index];
            }
        };

        recurse = [&]() -> void {
            if(current_part == "**") {
                for(auto& entry : std::filesystem::directory_iterator(current_path)) {
                    push_part();

                    const std::filesystem::path& entry_path = entry.path();
                    std::filesystem::path relative_path = std::filesystem::relative(entry_path, current_path);

                    current_path /= relative_path;

                    if(std::filesystem::is_directory(current_path)) {
                        for(auto& entry : std::filesystem::directory_iterator(current_path)) {
                            if(entry.is_directory()) {
                                current_path = entry.path();
                                recurse();
                                current_path = current_path.parent_path();
                            } else if(entry.is_regular_file() && current_part.starts_with('*')) {
                                std::string_view filename_pattern = current_part;
                                filename_pattern.remove_prefix(1); // remove the first character, which is a '*'

                                if(entry.path().filename().string().ends_with(filename_pattern)) {
                                    results.push_back(lavi::lang::object::instantiate(interpreter, interpreter->PathClass, std::move(entry.path())));
                                }
                            }
                        }
                    } // else if(std::filesystem::is_regular_file(current_path)) {
                    //     std::string_view filename_pattern = current_part;
                    //     filename_pattern.remove_prefix(1); // remove the first character, which is a '*'

                    //     if(current_path.filename().string().ends_with(filename_pattern)) {
                    //         results.push_back(lavi::lang::object::instantiate(interpreter, interpreter->PathClass, current_path));
                    //     }
                    // }

                    current_path = current_path.parent_path();
                    pop_part();
                }
            } else if(current_part.starts_with('*')) {
                if(current_part == "*") {
                    for(auto& entry : std::filesystem::directory_iterator(current_path)) {
                        // if(entry.is_directory()) {
                        //     current_path = entry.path();
                        //     recurse();
                        //     current_path = current_path.parent_path();
                        // } else if(entry.is_regular_file() && current_part.starts_with('*')) {
                            std::string_view filename_pattern = current_part;
                            filename_pattern.remove_prefix(1); // remove the first character, which is a '*'

                            if(entry.path().filename().string().ends_with(filename_pattern)) {
                                results.push_back(lavi::lang::object::instantiate(interpreter, interpreter->PathClass, std::move(entry.path())));
                            }
                        //}
                    }
                } else {
                    std::string_view filename_pattern = current_part;
                    filename_pattern.remove_prefix(1); // remove the first character, which is a '*'
                    for(auto& entry : std::filesystem::directory_iterator(current_path)) {
                        if(entry.is_regular_file() && entry.path().filename().string().ends_with(filename_pattern)) {
                            results.push_back(lavi::lang::object::instantiate(interpreter, interpreter->PathClass, std::move(entry.path())));
                        }
                    }
                }
            } else {
                current_path /= current_part;
                if(std::filesystem::exists(current_path) && std::filesystem::is_regular_file(current_path)) {
                    results.push_back(lavi::lang::object::instantiate(interpreter, interpreter->PathClass, std::move(current_path)));
                }
                push_part();
            }
        };

        for(const auto& part : parts) {
            current_part = part;
            recurse();
        }

        return lavi::lang::object::instantiate(interpreter, interpreter->ArrayClass, std::move(results));
    });

    DirectoryClass->functions["exists?"] = std::make_shared<lavi::lang::function>("exists?", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
        std::filesystem::path path;
        std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];
        if(path_object->cls == interpreter->StringClass) {
            path = path_object->as<std::string>();
        } else if(path_object->cls == interpreter->PathClass) {
            path = path_object->as<std::filesystem::path>();
        } else {
            throw std::runtime_error("invalid path");
        }
        if(std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            return lavi::lang::api::to_object(interpreter, true);
        } else {
            return lavi::lang::api::to_object(interpreter, false);
        }
    });

    DirectoryClass->functions["create"] = std::make_shared<lavi::lang::function>("create", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
        std::filesystem::path path;
        std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];
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

    DirectoryClass->functions["home"] = std::make_shared<lavi::lang::function>("home", [](lavi::lang::interpreter* interpreter) {
        std::filesystem::path path = std::filesystem::path(std::getenv("HOME"));
        if(path.empty()) {
            throw std::runtime_error("Unable to retrieve home directory");
        }
        return lavi::lang::object::instantiate(interpreter, interpreter->PathClass, std::move(path));
    });

    DirectoryClass->functions["current"] = std::make_shared<lavi::lang::function>("current", [DirectoryClass](lavi::lang::interpreter* interpreter) {
        std::filesystem::path path = std::filesystem::current_path();
        if(path.empty()) {
            throw std::runtime_error("Unable to retrieve current directory");
        }
        return lavi::lang::object::instantiate(interpreter, DirectoryClass, std::move(path));
    });

    return DirectoryClass;
}