#include <filesystem>
#include <fstream>

#include <lavi/lang/lang.hpp>
#include <lavi/lang/api.hpp>
#include <lavi/lang/interpreter.hpp>
#include <lavi/lang/file.hpp>

std::filesystem::path to_path(std::shared_ptr<lavi::lang::object> path_object)
{
  if(path_object->klass == lavi::lang::string_class) {
    return path_object->as<std::string>();
  } else if(path_object->klass == lavi::lang::path_class) {
    return path_object->as<std::filesystem::path>();
  } else {
    throw std::runtime_error("invalid path");
  }
}

void create_file_class()
{
  lavi::lang::file_class = lavi::lang::klass::create_builtin("File");

  lavi::lang::file_class->instance_functions["write"] = std::make_shared<lavi::lang::function>("write", std::initializer_list<std::string>{"content"}, [](lavi::lang::interpreter* interpreter) {
    std::shared_ptr<lavi::lang::object> self = interpreter->current_context->self;
    std::shared_ptr<lavi::lang::object> content_object = interpreter->current_context->positional_params[0];
    std::string content = content_object->as<std::string>();

    lavi::lang::file& file = self->as<lavi::lang::file>();
    file.write(content);

    return nullptr;
  });

  lavi::lang::file_class->functions["exists?"] = std::make_shared<lavi::lang::function>("exists?", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
    std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];
    std::filesystem::path path = to_path(path_object);

    bool exists = lavi::lang::file::exists(path);
    return lavi::lang::api::to_object(interpreter, exists);
  });

  lavi::lang::file_class->functions["create"] = std::make_shared<lavi::lang::function>("create", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
    std::filesystem::path path;
    std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];

    path = to_path(path_object);

    auto file = lavi::lang::file::create(path);
    return lavi::lang::api::to_object(interpreter, file);
  });

  lavi::lang::file_class->functions["read"] = std::make_shared<lavi::lang::function>("read", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
    std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];
    std::filesystem::path path = to_path(path_object);

    std::string content = lavi::lang::file::read(path);
    return lavi::lang::api::to_object(interpreter, content);
  });

  lavi::lang::file_class->functions["read_all_lines"] = std::make_shared<lavi::lang::function>("read_all_lines", std::initializer_list<std::string>{"path"}, [](lavi::lang::interpreter* interpreter) {
    std::shared_ptr<lavi::lang::object> path_object = interpreter->current_context->positional_params[0];
    std::filesystem::path path = to_path(path_object);
    std::vector<std::shared_ptr<lavi::lang::object>> line_objects;

    auto lines = lavi::lang::file::read_lines(path);

    for(const auto& line : lines) {
      line_objects.push_back(lavi::lang::api::to_object(interpreter, line));
    }

    return lavi::lang::api::to_object(interpreter, line_objects);
  });
}