#include <lavi/lang/function.hpp>
#include <lavi/lang/object.hpp>
#include <lavi/lang/class.hpp>

lavi::lang::fn_parameter::fn_parameter(std::string_view __name)
{
    name.reserve(__name.size());
    lavi::lang::lexer lexer("", std::string(__name));
    lexer.tokenize();
    if(lexer.tokens().size() == 1) {
        name = __name;
        return;
    }

    lavi::lang::parser parser;
    auto node = parser.parse_all(lexer).childrens().front();

    switch(node.type()) {
        case lavi::lang::parser::ast_node_type::ast_node_declname:
            name = node.token().content;
            break;
        case lavi::lang::parser::ast_node_type::ast_node_pair:
            name = node.child_content_from_type(lavi::lang::parser::ast_node_type::ast_node_declname);
            named = true;
            default_value_node = *node.child_from_type(lavi::lang::parser::ast_node_type::ast_node_valuedecl);
            break;
        default:
            throw std::runtime_error("Invalid parameter type: " + std::to_string(static_cast<int>(node.type())));
    }
}

void lavi::lang::function::init_params(std::vector<std::string> __params)
{
    for(auto& param : __params) {
        fn_parameter fn_param(std::move(param));
        if(fn_param.named) {
            named_params.push_back(std::move(fn_param));
        } else {
            positional_params.push_back(std::move(fn_param));
        }
    }
}
