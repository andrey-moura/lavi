#include <filesystem>
#include <iostream>

#include <lavi/lang/lang.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/extension.hpp>
#include <lavi/lang/api.hpp>

void create_path_class()
{
    lavi::lang::path_class = lavi::lang::klass::create_builtin("Path");

    lavi::lang::path_class->instance_functions["temp"] = std::make_shared<lavi::lang::function>("temp", [](lavi::lang::interpreter* interpreter) {
        return lavi::lang::object::create(interpreter, lavi::lang::path_class, std::move(std::filesystem::temp_directory_path()));
    });

    lavi::lang::path_class->instance_functions["init"] = std::make_shared<lavi::lang::function>("init", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
        auto object = interpreter->current_context->self;
        object->set_native<std::filesystem::path>(std::move(std::filesystem::path(interpreter->current_context->positional_params[0]->as<std::string>())));

        return nullptr;
    });

    lavi::lang::path_class->instance_functions["to_string"] = std::make_shared<lavi::lang::function>("to_string", [](lavi::lang::interpreter* interpreter) {
        return lavi::lang::api::to_object(interpreter, std::move(interpreter->current_context->self->as<std::filesystem::path>().string()));
    });

    lavi::lang::path_class->instance_functions["inspect"] = std::make_shared<lavi::lang::function>("inspect", [](lavi::lang::interpreter* interpreter) {
        return lavi::lang::api::call(
            interpreter,
            "inspect",
            lavi::lang::api::to_object(interpreter, std::move(interpreter->current_context->self->as<std::filesystem::path>().string()))
        );
    });

    lavi::lang::path_class->instance_functions["exists?"] = std::make_shared<lavi::lang::function>("exists?", [](lavi::lang::interpreter* interpreter) {
        std::filesystem::path& path = interpreter->current_context->self->as<std::filesystem::path>();

        if(std::filesystem::exists(path)) {
            return lavi::lang::api::to_object(interpreter, true);
        }

        return lavi::lang::api::to_object(interpreter, false);
    });

    lavi::lang::path_class->instance_functions["/="] = std::make_shared<lavi::lang::function>("/=", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
        std::filesystem::path& path = interpreter->current_context->self->as<std::filesystem::path>();
        path /= interpreter->current_context->positional_params[0]->as<std::string>();

        return interpreter->current_context->self->shared_from_this();
    });

    lavi::lang::path_class->instance_functions["/"] = std::make_shared<lavi::lang::function>("/", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
        std::filesystem::path path = interpreter->current_context->self->as<std::filesystem::path>() / interpreter->current_context->positional_params[0]->as<std::string>();
        
        return lavi::lang::object::create(interpreter, lavi::lang::path_class, std::move(path));
    });
    
    lavi::lang::path_class->functions["set_current"] = std::make_shared<lavi::lang::function>("set_current", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
        std::filesystem::path path;
        std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];

        if(path_object->klass == lavi::lang::string_class) {
            path = path_object->as<std::string>();
        } else if(path_object->klass == lavi::lang::path_class) {
            path = path_object->as<std::filesystem::path>();
        } else {
            throw std::runtime_error("invalid path");
        }

        std::filesystem::current_path(path);

        return nullptr;
    });
}