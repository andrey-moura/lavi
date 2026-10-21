#include <cctype>

#include <lavi/lang/function.hpp>
#include <lavi/lang/object.hpp>
#include <lavi/lang/class.hpp>

static lavi::lang::parser::ast_node parse_parameter(std::string_view source)
{
    lavi::lang::lexer lexer("", std::string(source));
    lexer.tokenize();

    lavi::lang::parser parser;
    return parser.parse_all(lexer).childrens().front();
}

// Parses a parameter declaration of a native function. It can be:
//   "name"          A positional parameter
//   "name = value"  A positional parameter with a default value
//   "name: value"   A named parameter with a default value
lavi::lang::fn_parameter::fn_parameter(std::string_view __name)
{
    // Positional parameter with a default value. '==' is not an assignment, but it's not valid in a declaration either.
    if(size_t equal_pos = __name.find('='); equal_pos != std::string_view::npos) {
        std::string_view name_view = __name.substr(0, equal_pos);

        while(name_view.size() && std::isspace((unsigned char)name_view.back())) {
            name_view.remove_suffix(1);
        }

        name = name_view;
        default_value_node = parse_parameter(__name.substr(equal_pos + 1));
        return;
    }

    lavi::lang::lexer lexer("", std::string(__name));
    lexer.tokenize();
    if(lexer.tokens().size() == 1) {
        name = __name;
        return;
    }

    auto node = parse_parameter(__name);

    switch(node.type()) {
        case lavi::lang::parser::ast_node_type::ast_node_declname:
            name = node.token().content;
            break;
        case lavi::lang::parser::ast_node_type::ast_node_pair:
            name = node.child_content_from_type(lavi::lang::parser::ast_node_type::ast_node_declname);
            named = true;
            // Keep the value itself, like the parameters of functions declared in Lavi
            default_value_node = node.child_from_type(lavi::lang::parser::ast_node_type::ast_node_valuedecl)->childrens().front();
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
