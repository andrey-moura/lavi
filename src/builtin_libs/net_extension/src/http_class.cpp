#include <lavi/lang/api.hpp>
#include <andy/net/http.hpp>

#include <iostream>

std::shared_ptr<lavi::lang::klass> http_class = nullptr;

extern std::shared_ptr<lavi::lang::klass> net_class;
extern std::shared_ptr<lavi::lang::klass> response_class;

void create_http_class()
{
    http_class = lavi::lang::klass::create_builtin("Net::HTTP");
    http_class->functions["get"] = std::make_shared<lavi::lang::function>("get", std::initializer_list<std::string>{ "url" }, [](lavi::lang::interpreter* interpreter) {
        const auto& url = interpreter->current_context->positional_params[0]->as<std::string>();

        auto response = lavi::net::http::get(url);
        int status_code = response.status_code;
        std::shared_ptr<lavi::lang::object> response_object = lavi::lang::object::create(interpreter, response_class, std::move(response));

        response_object->variables["status_code"] = lavi::lang::api::to_object(interpreter, status_code);
        return response_object;
    });
}