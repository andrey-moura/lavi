#include <lavi/lang/api.hpp>
#include <andy/net/http.hpp>

#include <iostream>

std::shared_ptr<lavi::lang::klass> response_class = nullptr;

void create_response_class()
{
    response_class = lavi::lang::klass::create_builtin("Net::HTTP::Response");

    response_class->instance_functions["text"] = std::make_shared<lavi::lang::function>("text", [](lavi::lang::interpreter* interpreter) {
        auto& response = interpreter->current_context->self->as<lavi::net::http::response>();
        std::string_view text = response.text();
        return lavi::lang::api::to_object(interpreter, text);
    });

    response_class->instance_functions["text?"] = std::make_shared<lavi::lang::function>("text?", [](lavi::lang::interpreter* interpreter) {
        auto& response = interpreter->current_context->self->as<lavi::net::http::response>();
        return lavi::lang::api::to_object(interpreter, response.is_text());
    });

    response_class->instance_functions["json"] = std::make_shared<lavi::lang::function>("json", [](lavi::lang::interpreter* interpreter) {
        auto& response = interpreter->current_context->self->as<lavi::net::http::response>();
        std::string_view content_type = response.headers["content-type"];
        if (!content_type.starts_with("application/json")) {
            throw std::runtime_error("Trying to access response body as JSON, but Content-Type is not application/json");
        }

        std::string_view json_text(reinterpret_cast<const char*>(response.raw_body.data()), response.raw_body.size());

        lavi::lang::lexer l("", std::string(json_text));
        l.tokenize();

        lavi::lang::parser p;
        auto node = p.parse_identifier_or_literal(l);

        return interpreter->node_to_object(node);
    });

    response_class->instance_functions["json?"] = std::make_shared<lavi::lang::function>("json?", [](lavi::lang::interpreter* interpreter) {
        auto& response = interpreter->current_context->self->as<lavi::net::http::response>();
        return lavi::lang::api::to_object(interpreter, response.headers["content-type"].starts_with("application/json"));
    });
}