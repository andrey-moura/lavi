#include <lavi/lang/parser.hpp>

#include <andy/file.hpp>
#include <andy/console.hpp>

#include <exception>
#include <iostream>
#include <regex>

#include <lavi/lang/object.hpp>
#include <lavi/lang/class.hpp>
#include <lavi/lang/function.hpp>

using namespace lavi;
using namespace lang;

lavi::lang::parser::parser()
{
}

lavi::lang::parser::ast_node extract_context(lavi::lang::lexer &lexer, lavi::lang::parser& parser, std::vector<std::string_view>* break_on_keywords = nullptr)
{
    lavi::lang::parser::ast_node output(lavi::lang::parser::ast_node_type::ast_node_context);
    while(true) {
        const lavi::lang::lexer::token& next_token = lexer.see_next();

        if(next_token.type == lavi::lang::lexer::token_delimiter) {
            if(next_token.content == "end") {
                // The token is consumed by the caller
                break;
            } else if (next_token.content == ";") {
                // We have to consume ';' here because we are expecting '}'. If there is a ';' just before a
                // '}', the call to parse_node will ignore ';' and throw an error at the '}' token.
                lexer.consume_token();
                continue;
            }
        } else if(next_token.type == lavi::lang::lexer::token_comment) {
            lexer.consume_token();
            continue;
        } else if(next_token.type == lavi::lang::lexer::token_type::token_eof) {
            throw lavi::lang::parser::unexpected_token_error(next_token, "end");
        } else if (next_token.type == lavi::lang::lexer::token_type::token_keyword && break_on_keywords) {
            bool should_break = false;
            for (const auto& keyword : *break_on_keywords) {
                if (next_token.content == keyword) {
                    // If we are breaking on a specific keyword, we stop here.
                    should_break = true;
                    break;
                }
            }
            if (should_break) {
                break;
            }
        }
        
        lavi::lang::parser::ast_node context_child = parser.parse_node(lexer);
        output.add_child(std::move(context_child));
    }
    return output;
}

lavi::lang::parser::ast_node parser::parse_node(lavi::lang::lexer &lexer)
{
    // We are at the middle of the source code.
    // What we can have in the middle of the source code:
    // - A comment (we can ignore it)
    // - A keyword
    // - An identifier
    // - A literal
    // - A delimiter (only ';', which is ignored or '{' which is the start of a context)
    // Anything else is an syntax error.

    // We need to see the next token to know what to do.

    const lavi::lang::lexer::token& token = lexer.see_next();

    switch (token.type)
    {
    case lavi::lang::lexer::token_type::token_comment:
        // Ignore the comment and return the next node
        lexer.consume_token();
        return parse_node(lexer);
        break;
    case lavi::lang::lexer::token_type::token_identifier:
    case lavi::lang::lexer::token_type::token_literal:
        return parse_identifier_or_literal(lexer);
        break;
    case lavi::lang::lexer::token_type::token_delimiter:
        return parse_delimiter(lexer);
    break;
    case lavi::lang::lexer::token_type::token_keyword:
        return parse_keyword(lexer);
        break;
    case lavi::lang::lexer::token_type::token_eof:
        return parse_eof(lexer);
        break;
    case lavi::lang::lexer::token_type::token_preprocessor:
        return parse_preprocessor(lexer);
        break;
    default:
        break;
    }
    
    // If none of the above, it is an error
    throw std::runtime_error(token.error_message_at_current_position("Unexpected token"));
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_all(lavi::lang::lexer &lexer)
{
    ast_node root_node(ast_node_type::ast_node_unit);

    do {
        const lavi::lang::lexer::token token = lexer.see_next();
        if(token.is_eof()) {
            break;
        }
        ast_node child = parse_node(lexer);
        root_node.add_child(std::move(child));
    } while(lexer.has_next_token());

    return root_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::extract_pair(lavi::lang::lexer &lexer)
{
    // If this function has been called, we are sure the next tokens are a pair. No need to check for errors.

    // Parse the key
    ast_node pair_node(ast_node_type::ast_node_pair);

    auto& key_token = lexer.next_token();

    if(key_token.type == lavi::lang::lexer::token_type::token_identifier) {
        key_token.type = lavi::lang::lexer::token_type::token_literal;
        key_token.kind = lavi::lang::lexer::token_kind::token_string;
        key_token.string_literal = key_token.content;
    }

    ast_node key_node = ast_node(ast_node_type::ast_node_declname);
    key_node.add_child(ast_node(std::move(key_token), ast_node_type::ast_node_valuedecl));

    pair_node.add_child(std::move(key_node));
    
    // Consume the ':' token
    lexer.next_token();

    // Extract the value
    ast_node value_node = parse_identifier_or_literal(lexer);
    value_node.set_type(ast_node_type::ast_node_valuedecl);
    pair_node.add_child(std::move(value_node));

    return pair_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::extract_fn_call_params(lavi::lang::lexer &lexer)
{
    ast_node params_node(ast_node_type::ast_node_fn_params);

    while(true) {
        auto& possible_yield = lexer.see_next();

        if(possible_yield.type == lavi::lang::lexer::token_type::token_keyword && possible_yield.content == "yield")
        {
            ast_node param_node = parse_keyword(lexer);
            params_node.add_child(std::move(param_node));
        } else {
            ast_node param_node = parse_identifier_or_literal(lexer);
            params_node.add_child(std::move(param_node));
        }

        auto& token = lexer.see_next();

        if(token.type == lexer::token_type::token_delimiter)
        {
            if(token.content == ":") {
                lexer.consume_token();

                ast_node named_param(ast_node_type::ast_node_valuedecl);
                ast_node key_node(std::move(params_node.childrens().back()));
                key_node.set_type(ast_node_type::ast_node_declname);
                named_param.add_child(std::move(key_node));
                params_node.childrens().pop_back();

                ast_node value_node = parse_identifier_or_literal(lexer);
                value_node.set_type(ast_node_type::ast_node_valuedecl);
                named_param.add_child(std::move(value_node));

                params_node.add_child(std::move(named_param));
            }

            if(lexer.see_next().content == ",") {
                lexer.consume_token();
            } else {
                break;
            }
        } else {
            break;
        }
    }

    return params_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_delimiter(lavi::lang::lexer &lexer)
{
    const lavi::lang::lexer::token& token = lexer.see_next();

    if(token.content == ";") {
        // ; in the middle of the code is considered a whitespace
        lexer.consume_token();
        return parse_node(lexer);
    } else if(token.content == "{") {
        // By now we assuming that every '{' in the middle of the code is a Hash declaration
        return parse_identifier_or_literal(lexer);
    }
    else {
        throw std::runtime_error(token.error_message_at_current_position("Unexpected delimiter"));
    }

    return ast_node(std::move(token), ast_node_type::ast_node_undefined);
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_eof(lavi::lang::lexer &lexer)
{
    lavi::lang::lexer::token token = lexer.next_token();
    return ast_node(std::move(token), ast_node_type::ast_node_undefined);
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_preprocessor(lavi::lang::lexer &lexer)
{
    lavi::lang::lexer::token token = lexer.next_token();

    // If the directive has not been removed by the preprocessor, it is probably in an invalid location
    throw std::runtime_error(token.error_message_at_current_position("Unexpected '" + std::string(token.content) + "' directive"));
}

static bool is_identifier_or_literal(const lavi::lang::lexer::token& token)
{
    return token.type == lavi::lang::lexer::token_type::token_identifier ||
           token.type == lavi::lang::lexer::token_type::token_literal;
}

static bool is_on_same_line(const lavi::lang::lexer::token& token, const lavi::lang::lexer::token& other)
{
    return token.start.line == other.start.line;
}

static bool is_function_call(const lavi::lang::lexer::token& token, const lavi::lang::lexer& lexer)
{
    if(token.type != lavi::lang::lexer::token_type::token_identifier) {
        return false;
    }
    
    auto& next_token = lexer.see_next();

    return next_token.type == lavi::lang::lexer::token_type::token_delimiter
            && next_token.content == "(";
}

static bool is_no_parentheses_function_call(const lavi::lang::lexer::token& token, const lavi::lang::lexer& lexer)
{
    if(token.type != lavi::lang::lexer::token_type::token_identifier) {
        return false;
    }

    auto& next_token = lexer.see_next();

    bool is_identifier_or_literal_or_yield = is_identifier_or_literal(next_token) || (next_token.type == lavi::lang::lexer::token_type::token_keyword && next_token.content == "yield");

    return is_identifier_or_literal_or_yield && is_on_same_line(token, next_token);
}

static bool is_any_function_call(const lavi::lang::lexer::token& token, const lavi::lang::lexer& lexer)
{
    return is_function_call(token, lexer) || is_no_parentheses_function_call(token, lexer);
}

static void extract_fn_yield_block_if_exists(lavi::lang::parser::ast_node& node, lavi::lang::parser& parser, const lavi::lang::lexer::token& token, lavi::lang::lexer& lexer)
{
    if(node.type() != lavi::lang::parser::ast_node_type::ast_node_fn_call &&
        node.type() != lavi::lang::parser::ast_node_type::ast_node_declname) {
        // Cant extract yield block from this node
        return;
    }

    auto& next_token = lexer.see_next();

    if(next_token.type == lavi::lang::lexer::token_type::token_identifier && next_token.content == "do") {
        lavi::lang::parser::ast_node yield_node(lavi::lang::parser::ast_node_type::ast_node_yield_block);

        auto& token = lexer.next_token(); // Consume the 'do' token
        auto& possible_block_params = lexer.see_next();

        if(possible_block_params.type == lavi::lang::lexer::token_type::token_delimiter && possible_block_params.content == "(") {
            // The block has parameters, we need to extract them before extracting the block itself
            // ex: items.map do (item) ... end
            lexer.consume_token(); // Consume the '(' token
            lavi::lang::parser::ast_node params_node = parser.extract_fn_call_params(lexer);
            yield_node.add_child(std::move(params_node));
            lexer.consume_token(); // Consume the ')' token
        }

        lavi::lang::parser::ast_node yield_context = extract_context(lexer, parser);
        yield_context.set_token(std::move(token));
        yield_context.set_end_token(lexer.next_token());

        yield_node.add_child(std::move(yield_context));

        node.add_child(std::move(yield_node));
    }
}

static lavi::lang::parser::ast_node chain_if_exists(lavi::lang::parser::ast_node& node, lavi::lang::parser& parser, lavi::lang::lexer& lexer)
{
    // if(chained_nodes.size() == 0) {
        extract_fn_yield_block_if_exists(node, parser, node.token(), lexer);
        // return node;
    // }

    std::vector<lavi::lang::parser::ast_node> chained_nodes;

    while(true) {
        const lavi::lang::lexer::token& next_token = lexer.see_next();

        if(next_token.type == lavi::lang::lexer::token_type::token_operator) {
            if(next_token.content == "]") {
                // Already handled in the array declaration
                break;
            }
            if(next_token.content == "." || next_token.content == "::") {
                lexer.consume_token(); // Consume the '.' token

                lavi::lang::parser::ast_node next_node = parser.parse_identifier_or_literal(lexer, false, { "class" });
                chained_nodes.push_back(std::move(next_node));
            } else {
                lavi::lang::parser::ast_node operator_node(lavi::lang::parser::ast_node_type::ast_node_fn_call);
                lavi::lang::lexer::token& operator_token = lexer.next_token();

                std::string matching;

                if(operator_token.content == "[") {
                    matching = "]";
                }

                // ++ and -- are unary operators
                if(operator_token.content != "++" && operator_token.content != "--") {
                    lavi::lang::parser::ast_node right_node = parser.parse_identifier_or_literal(lexer);

                    lavi::lang::parser::ast_node params_node(lavi::lang::parser::ast_node_type::ast_node_fn_params);
                    params_node.add_child(std::move(right_node));

                    if(matching.size()) {
                        lavi::lang::lexer::token& matching_token = lexer.next_token();

                        if(matching_token.content != matching) {
                            throw std::runtime_error(matching_token.error_message_at_current_position("No matching '" + std::string(matching) + "' found for '" + std::string(operator_node.token().content) + "'"));
                        }

                        operator_token.merge(matching_token);
                    }

                    operator_node.add_child(std::move(params_node));
                }

                operator_node.add_child(lavi::lang::parser::ast_node(std::move(operator_token), lavi::lang::parser::ast_node_type::ast_node_declname));

                chained_nodes.push_back(std::move(operator_node));
            }
        } else {
            break;
        }
    }

    chained_nodes.insert(chained_nodes.begin(), std::move(node));

    for(size_t i = 0; i < chained_nodes.size(); i++) {
        lavi::lang::parser::ast_node& node = chained_nodes[i];

        if(i < chained_nodes.size() - 1) {
            lavi::lang::parser::ast_node& next_node = chained_nodes[i + 1];

            if(node.type() == lavi::lang::parser::ast_node_type::ast_node_declname && next_node.type() == lavi::lang::parser::ast_node_type::ast_node_declname) {
                if(next_node.token().content == "new") {
                    lavi::lang::parser::ast_node fn_call_node(lavi::lang::parser::ast_node_type::ast_node_fn_call);
                    fn_call_node.add_child(std::move(next_node));

                    next_node = std::move(fn_call_node);
                }
            }

            lavi::lang::parser::ast_node object_node(lavi::lang::parser::ast_node_type::ast_node_fn_object);
            object_node.add_child(std::move(node));

            next_node.add_child(std::move(object_node));
        }
    }

    auto& last_node = chained_nodes[chained_nodes.size() - 1];

    extract_fn_yield_block_if_exists(last_node, parser, last_node.token(), lexer);

    return last_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_identifier_or_literal(lavi::lang::lexer &lexer, bool chain, std::vector<std::string_view> keyword)
{
    const lavi::lang::lexer::token& token = lexer.see_next();

    lavi::lang::lexer::token identifier_or_literal;

    switch(token.type) {
        case lavi::lang::lexer::token_type::token_literal:
        case lavi::lang::lexer::token_type::token_identifier: {
            auto possible_colon = lexer.see_next(1);
            if(possible_colon.type == lavi::lang::lexer::token_type::token_delimiter && possible_colon.content == ":") {
                return extract_pair(lexer);
            }
            identifier_or_literal = std::move(lexer.next_token());
            break;
        }
        case lavi::lang::lexer::token_type::token_operator:
            // The lexer sees array as operator and we need to handle it here
            if(token.content == "[") {
                ast_node array_node(ast_node_type::ast_node_arraydecl);
                lexer.consume_token(); // Consume the '[' token
                while(true) {
                    ast_node value_node = parse_identifier_or_literal(lexer);

                    if(value_node.type() != ast_node_type::ast_node_valuedecl && value_node.type() != ast_node_type::ast_node_hashdecl && value_node.type() != ast_node_type::ast_node_arraydecl) {
                        throw std::runtime_error(token.error_message_at_current_position("Expected value in array"));
                    }

                    array_node.add_child(std::move(value_node));

                    const lavi::lang::lexer::token& comma_or_closing = lexer.next_token();

                    if(comma_or_closing.type == lavi::lang::lexer::token_type::token_delimiter && comma_or_closing.content == ",") {
                        if(lexer.see_next().content == "]") {
                            lexer.consume_token();
                            break;
                        }
                    } else {
                        if(comma_or_closing.type == lavi::lang::lexer::token_type::token_operator && comma_or_closing.content == "]") {
                            break;
                        } else {
                            throw std::runtime_error(token.error_message_at_current_position("Expected ',' or ']'"));
                        }
                    }
                }

                return array_node;
            } else if (token.content == "!") {
                identifier_or_literal = std::move(lexer.next_token());

                ast_node unary_op(ast_node_type::ast_node_fn_call);
                unary_op.add_child(std::move(ast_node(std::move(identifier_or_literal), ast_node_type::ast_node_declname)));

                ast_node object_node(ast_node_type::ast_node_fn_object);
                object_node.add_child(parse_identifier_or_literal(lexer));

                unary_op.add_child(std::move(object_node));

                return unary_op;
            } else if(token.content == "[]") {
                // empty array declaration
                ast_node array_node(ast_node_type::ast_node_arraydecl);
                lexer.consume_token(); // Consume the '[]' token
                return array_node;
            }
            else {
                throw std::runtime_error(token.error_message_at_current_position("Unexpected operator"));
            }
        break;
        case lavi::lang::lexer::token_type::token_delimiter: {
            // Can be a map {}

            if(token.content == "{") {
                ast_node map_node(ast_node_type::ast_node_hashdecl);

                // The token was seen, so we need to consume it
                lexer.consume_token();

                while(true) {
                    ast_node pair_node = parse_identifier_or_literal(lexer);

                    if(pair_node.type() != lavi::lang::parser::ast_node_type::ast_node_pair) {
                        throw std::runtime_error(token.error_message_at_current_position("Expected key-value pair in map declaration"));
                    }

                    map_node.add_child(std::move(pair_node));

                    const lavi::lang::lexer::token& comma_token = lexer.next_token();

                    if(comma_token.content == ",") {
                        if(lexer.see_next().content == "}") {
                            lexer.next_token();
                            break;
                        }
                    } else {
                        if(comma_token.content == "}") {
                            break;
                        } else if(comma_token.is_eof()) {
                            throw std::runtime_error(comma_token.error_message_at_current_position("Expected '}'"));
                        } else {
                            continue;
                        }
                    }
                }

                return map_node;
            } else if(token.content == "(") {
                // Expressions like (1 + 2)
                lexer.consume_token(); // Consume the '(' token
                ast_node expression_node = parse_identifier_or_literal(lexer);
                lexer.consume_token(); // Consume the ')' token
                return chain_if_exists(expression_node, *this, lexer);
            }
            else {
                throw std::runtime_error(token.error_message_at_current_position("Unexpected delimiter"));
            }
        }
        break;
        case lavi::lang::lexer::token_type::token_keyword:
            if(keyword.size()) {
                if(std::find(keyword.begin(), keyword.end(), token.content) != keyword.end()) {
                    identifier_or_literal = std::move(lexer.next_token());
                    break;
                }
            }
        default:
            throw std::runtime_error(token.error_message_at_current_position("Expected identifier or literal"));
            break;
    }

    ast_node identifier_or_literal_node;
    const auto& next_token = lexer.see_next();

    if(is_any_function_call(identifier_or_literal, lexer)) {
        ast_node fn_node(ast_node_type::ast_node_fn_call);
        fn_node.add_child(std::move(ast_node(std::move(identifier_or_literal), ast_node_type::ast_node_declname)));

        if(next_token.type == lavi::lang::lexer::token_type::token_delimiter && next_token.content == "(") {
            lexer.consume_token(); // Consume the '(' token

            if(auto next_token = lexer.see_next(); next_token.type == lavi::lang::lexer::token_type::token_delimiter && next_token.content == ")") {
                // No parameters, just a closing parenthesis
                // Consumes the ')' token
                lexer.consume_token();
            } else {
                ast_node params_node = extract_fn_call_params(lexer);
                fn_node.add_child(std::move(params_node));

                const auto& possible_closing = lexer.see_next();

                if(possible_closing.type == lavi::lang::lexer::token_type::token_delimiter && possible_closing.content == ")") {
                    // Consume the ')' token
                    lexer.consume_token();
                } else {
                    throw std::runtime_error(possible_closing.error_message_at_current_position("Expected ')'"));
                }
            }
        } else if (auto& next_token = lexer.see_next();
                  (next_token.type == lavi::lang::lexer::token_type::token_identifier && next_token.content != "do") ||
                  next_token.type == lavi::lang::lexer::token_type::token_literal ||
                    (next_token.type == lavi::lang::lexer::token_type::token_keyword && next_token.content == "yield")) {
            // fn call with literal or identifier
            ast_node params_node = extract_fn_call_params(lexer);
            fn_node.add_child(std::move(params_node));
        }
        identifier_or_literal_node = std::move(fn_node);
    }
    else {
        ast_node_type node_type;
    
        if(identifier_or_literal.type == lavi::lang::lexer::token_type::token_literal) {
            node_type = ast_node_type::ast_node_valuedecl;
        } else {
            node_type = ast_node_type::ast_node_declname;
        }

        identifier_or_literal_node = lavi::lang::parser::ast_node(std::move(identifier_or_literal), node_type);
    }

    if(identifier_or_literal_node.token().type == lavi::lang::lexer::token_type::token_literal &&
        identifier_or_literal_node.token().kind == lavi::lang::lexer::token_kind::token_interpolated_string)
    {
        identifier_or_literal_node.token().kind = lavi::lang::lexer::token_kind::token_string;
        ast_node interpolated_node(ast_node_type::ast_node_interpolated_string);
        interpolated_node.add_child(std::move(identifier_or_literal_node));

        while(true)
        {
            const auto& possible_end = lexer.see_next();
            if(possible_end.type == lavi::lang::lexer::token_type::token_delimiter &&
                possible_end.delimiter == lavi::lang::lexer::token_delimiter_type::delimiter_end)
            {
                lexer.consume_token();
                break;
            }

            // The ',' token is generated by the lexer to separate the string parts from the expressions

            const auto& possible_comma = lexer.see_next();
            if(possible_comma.type != lavi::lang::lexer::token_type::token_delimiter || possible_comma.delimiter != lavi::lang::lexer::token_delimiter_type::delimiter_comma) {
                throw std::runtime_error(possible_comma.error_message_at_current_position("Expected ',' in interpolated string"));
            }

            lexer.consume_token();

            ast_node child_node = parse_identifier_or_literal(lexer);
            interpolated_node.add_child(std::move(child_node));
        }

        identifier_or_literal_node = interpolated_node;
    }

    if(!chain) {
        extract_fn_yield_block_if_exists(identifier_or_literal_node, *this, identifier_or_literal_node.token(), lexer);
        return identifier_or_literal_node;
    }

    return chain_if_exists(identifier_or_literal_node, *this, lexer);
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword(lavi::lang::lexer &lexer)
{
    const lavi::lang::lexer::token& token = lexer.see_next();

    static std::map<std::string_view, lavi::lang::parser::ast_node(lavi::lang::parser::*)(lavi::lang::lexer&)> keyword_parsers = {
        { "class",      &lavi::lang::parser::parse_keyword_class    },
        { "var",       &lavi::lang::parser::parse_keyword_var       },
        { "fn" ,       &lavi::lang::parser::parse_keyword_function  },
        { "return",    &lavi::lang::parser::parse_keyword_return    },
        { "if",        &lavi::lang::parser::parse_keyword_if        },
        { "unless",    &lavi::lang::parser::parse_keyword_if        },
        { "loop",      &lavi::lang::parser::parse_keyword_loop      },
        { "namespace", &lavi::lang::parser::parse_keyword_namespace },
        { "break",     &lavi::lang::parser::parse_keyword_break     },
        { "static",    &lavi::lang::parser::parse_keyword_static    },
        { "yield",     &lavi::lang::parser::parse_keyword_yield     },
        { "within",    &lavi::lang::parser::parse_keyword_within    },
        { "throw",     &lavi::lang::parser::parse_keyword_throw     },
        { "try",       &lavi::lang::parser::parse_keyword_try       },
        { "enum",      &lavi::lang::parser::parse_keyword_enum      },
    };

    auto keyword_parser = keyword_parsers.find(token.content);

    if(keyword_parser == keyword_parsers.end()) {
        throw std::runtime_error(token.error_message_at_current_position("Unexpected keyword"));
    }

    return (this->*keyword_parser->second)(lexer);
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_class(lavi::lang::lexer &lexer) {
    ast_node class_node(ast_node_type::ast_node_classdecl);

    class_node.add_child(std::move(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_decltype)));

    const lavi::lang::lexer::token& identifier_token = lexer.see_next();

    if(identifier_token.type != lexer::token_type::token_identifier) {
        throw std::runtime_error(identifier_token.error_message_at_current_position("Expected class name after 'class'"));
    }

    class_node.add_child(std::move(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_declname)));

    const lavi::lang::lexer::token& extends_or_context_token = lexer.see_next();

    if(extends_or_context_token.type == lavi::lang::lexer::token_identifier && extends_or_context_token.content == "extends") {
        ast_node base_class_node(ast_node_type::ast_node_classdecl_base);
        base_class_node.add_child(std::move(ast_node(extends_or_context_token, ast_node_type::ast_node_decltype)));
        lexer.consume_token(); // Consume the 'extends' or ':' token

        const lavi::lang::lexer::token& baseclass_token = lexer.see_next();

        if(baseclass_token.type != lexer::token_type::token_identifier) {
            throw std::runtime_error(baseclass_token.error_message_at_current_position("Expected identifier as base class name"));
        }

        ast_node base_class_name_node = parse_identifier_or_literal(lexer);

        base_class_node.add_child(std::move(base_class_name_node));

        class_node.add_child(base_class_node);
    }

    ast_node class_context = extract_context(lexer, *this);
    class_node.add_child(std::move(class_context));
    class_node.set_end_token(lexer.next_token());

    return class_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_var(lavi::lang::lexer &lexer){
    ast_node var_node(ast_node_type::ast_node_vardecl);
    var_node.add_child(std::move(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_decltype)));

    lavi::lang::lexer::token identifier_token = lexer.see_next();

    if(identifier_token.type != lexer::token_type::token_identifier) {
        throw std::runtime_error(identifier_token.error_message_at_current_position("Expected variable name after 'var'"));
    }

    var_node.add_child(std::move(ast_node(std::move(identifier_token), ast_node_type::ast_node_declname)));

    const lavi::lang::lexer::token& equal_token = lexer.see_next(1);

    if(equal_token.type == lexer::token_type::token_operator && equal_token.content == "=") {
        // There is an '=' token, extract as a function call
        var_node.add_child(parse_node(lexer));
    } else {
        // No '=' token, consume the name token.
        lexer.consume_token();
    }

    return var_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_function(lavi::lang::lexer &lexer) {
    ast_node method_node(ast_node_type::ast_node_fn_decl);
    method_node.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_decltype));

    const lavi::lang::lexer::token& identifier_token = lexer.see_next();

    switch(identifier_token.type)
    {
        case lexer::token_type::token_identifier:
            // Simply use it as the function name
            break;
        default:
            throw std::runtime_error(identifier_token.error_message_at_current_position("Expected function name after 'function'"));
            break;
    }

    method_node.add_child(std::move(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_declname)));

    const lavi::lang::lexer::token& parenthesis_token = lexer.see_next();

    if(parenthesis_token.content == "(") {
        lexer.consume_token(); // Consume the '(' token

        ast_node params_node(ast_node_type::ast_node_fn_params);

        while(true) {
            const lavi::lang::lexer::token& identifier_or_parenthesis = lexer.see_next();

            if(identifier_or_parenthesis.type == lexer::token_type::token_identifier) {
                auto next_token = lexer.see_next(1);

                ast_node param_node(ast_node_type::ast_node_undefined);

                if (next_token.type == lexer::token_type::token_delimiter && next_token.content == ":") {
                    param_node = std::move(extract_pair(lexer));
                } else {
                    param_node = ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_declname);
                }

                auto possible_equal = lexer.see_next();

                if(possible_equal.type == lexer::token_type::token_operator && possible_equal.content == "=") {
                    lexer.consume_token(); // Consume the '=' token

                    ast_node default_value_node(ast_node_type::ast_node_valuedecl);
                    default_value_node.add_child(parse_identifier_or_literal(lexer));

                    param_node.add_child(std::move(default_value_node));
                }

                params_node.add_child(std::move(param_node));

                auto possible_comma = lexer.see_next();

                if(possible_comma.type == lexer::token_type::token_delimiter && possible_comma.content == ",") {
                    lexer.consume_token(); // Consume the ',' token
                    continue;
                }
            } else if(identifier_or_parenthesis.type == lexer::token_type::token_delimiter && identifier_or_parenthesis.content == ")") {
                lexer.consume_token(); // Consume the ')' token
                break;
            }
            else {
                throw std::runtime_error(identifier_or_parenthesis.error_message_at_current_position("Expected parameter name"));
            }
        }

        method_node.add_child(std::move(params_node));
    }

    ast_node fn_context = extract_context(lexer, *this);
    method_node.add_child(std::move(fn_context));
    method_node.set_end_token(lexer.next_token());

    return method_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_return(lavi::lang::lexer &lexer) {
    ast_node return_node(std::move(lexer.next_token()), ast_node_type::ast_node_fn_return);

    auto possible_return_value = lexer.see_next();

    if(possible_return_value.start.line != return_node.token().start.line) {
        return return_node; // No return value, just return the return node
    }

    if(possible_return_value.type == lexer::token_type::token_keyword) {
        throw std::runtime_error(possible_return_value.error_message_at_current_position("Unexpected keyword after 'return'"));
    }

    return_node.add_child(std::move(parse_identifier_or_literal(lexer)));

    return return_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_if(lavi::lang::lexer &lexer){
    ast_node if_node(ast_node_type::ast_node_conditional);
    if_node.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_decltype));

    ast_node condition_node(ast_node_type::ast_node_condition);
    condition_node.add_child(std::move(parse_identifier_or_literal(lexer)));

    if_node.add_child(std::move(condition_node));

    static std::vector<std::string_view> valid_following_tokens = { "else" };
    ast_node if_context = extract_context(lexer, *this, &valid_following_tokens);
    if_node.add_child(std::move(if_context));
    
    lavi::lang::lexer::token token = lexer.next_token();

    if_node.set_end_token(token); // Consume the closing 'end' or 'else'

    // Check if there is an else
    if(token.type == lavi::lang::lexer::token_type::token_keyword && token.content == "else") {
        // Check if it is an else if
        const lavi::lang::lexer::token& next_token = lexer.see_next();

        if(next_token.type == lavi::lang::lexer::token_type::token_keyword && next_token.content == "if") {
            ast_node else_node = parse_keyword_if(lexer);
            if_node.add_child(std::move(else_node));
        } else {
            ast_node else_node(ast_node_type::ast_node_else);
            ast_node else_context = extract_context(lexer, *this);
            else_node.add_child(std::move(else_context));
            else_node.set_end_token(lexer.next_token());
            if_node.add_child(std::move(else_node));
        }
    }

    return if_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_namespace(lavi::lang::lexer &lexer) {
    ast_node namespace_node(ast_node_type::ast_node_classdecl);
    namespace_node.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_decltype));

    const lavi::lang::lexer::token& identifier_token = lexer.see_next();

    if(identifier_token.type != lexer::token_type::token_identifier) {
        throw std::runtime_error(identifier_token.error_message_at_current_position("Expected namespace name after 'namespace'"));
    }

    namespace_node.add_child(std::move(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_declname)));

    ast_node namespace_context = extract_context(lexer, *this);
    namespace_node.add_child(std::move(namespace_context));
    namespace_node.set_end_token(lexer.next_token()); // Consume the closing 'end'

    return namespace_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_loop(lavi::lang::lexer& lexer)
{
    lexer.consume_token(); // Consume the 'loop' keyword

    static std::map<std::string_view, lavi::lang::parser::ast_node(lavi::lang::parser::*)(lavi::lang::lexer&)> loop_parsers = {
        { "while",   &lavi::lang::parser::parse_keyword_while   },
        { "times",   &lavi::lang::parser::parse_keyword_for },
        { "until",   &lavi::lang::parser::parse_keyword_while   }
    };

    const lavi::lang::lexer::token& next_token = lexer.see_next();

    if(next_token.type == lavi::lang::lexer::token_type::token_identifier) {
        auto loop_parser = loop_parsers.find(next_token.content);

        if(loop_parser != loop_parsers.end()) {
            // Call the appropriate loop parser
            return (this->*loop_parser->second)(lexer);
        }
    }

    throw std::runtime_error(next_token.error_message_at_current_position("Expected 'while', 'for', 'foreach' or 'times' after 'loop'"));
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_for(lavi::lang::lexer &lexer)
{
    ast_node for_node(ast_node_type::ast_node_for);

    const lavi::lang::lexer::token& times_token = lexer.next_token();
    for_node.add_child(ast_node(std::move(times_token), ast_node_type::ast_node_decltype));

    ast_node value_node = parse_identifier_or_literal(lexer);
    for_node.add_child(std::move(value_node));

    ast_node context_node = extract_context(lexer, *this);
    for_node.add_child(std::move(context_node));
    for_node.set_end_token(lexer.next_token()); // Consume the closing 'end'
    return for_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_while(lavi::lang::lexer &lexer)
{
    ast_node while_node(ast_node_type::ast_node_while);
    while_node.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_decltype));

    ast_node condition_node(ast_node_type::ast_node_condition);
    condition_node.add_child(std::move(parse_identifier_or_literal(lexer)));

    while_node.add_child(std::move(condition_node));

    ast_node while_context = extract_context(lexer, *this);
    while_node.add_child(std::move(while_context));
    while_node.set_end_token(lexer.next_token()); // Consume the closing 'end'

    return while_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_break(lavi::lang::lexer &lexer)
{
    ast_node break_node(ast_node_type::ast_node_break);
    break_node.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_decltype));

    return break_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_static(lavi::lang::lexer &lexer)
{
    const lavi::lang::lexer::token& static_token = lexer.next_token();

    const lavi::lang::lexer::token& next_token = lexer.see_next();

    if(next_token.type == lavi::lang::lexer::token_type::token_keyword) {
        if(next_token.content == "fn") {
            ast_node node = parse_keyword_function(lexer);
            node.add_child(ast_node(std::move(static_token), ast_node_type::ast_node_declstatic));

            return node;
        } else if(next_token.content == "var") {
            ast_node node = parse_keyword_var(lexer);
            node.add_child(ast_node(std::move(static_token), ast_node_type::ast_node_declstatic));

            return node;
        }
    }
    
    throw std::runtime_error(next_token.error_message_at_current_position("Expected keyword 'fun' or 'var' after 'static'"));
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_within(lavi::lang::lexer &lexer)
{
    lexer.consume_token(); // Consume the 'within' token
    const lavi::lang::lexer::token& token = lexer.next_token();
    ast_node object_node(ast_node_type::ast_node_fn_object);
    object_node.add_child(ast_node(std::move(token), ast_node_type::ast_node_declname));

    ast_node within_node = extract_context(lexer, *this);
    within_node.add_child(std::move(object_node));
    within_node.set_end_token(lexer.next_token()); // Consume the closing 'end'
    return within_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_yield(lavi::lang::lexer &lexer)
{
    lavi::lang::parser::ast_node node(std::move(lexer.next_token()), ast_node_type::ast_node_yield);

    auto next_token = lexer.see_next();

    if(next_token.type == lavi::lang::lexer::token_type::token_delimiter && next_token.content == "(") {
        lexer.consume_token(); // Consume the '(' token
        node.add_child(extract_fn_call_params(lexer));
        lexer.consume_token(); // Consume the ')' token
    }

    return node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_throw(lavi::lang::lexer &lexer)
{
    // throw [object_to_throw]

    lavi::lang::parser::ast_node node(std::move(lexer.next_token()), ast_node_type::ast_node_throw);
    auto next_node = parse_identifier_or_literal(lexer);
    node.add_child(std::move(next_node));

    return node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_try(lavi::lang::lexer &lexer)
{
    /*
    try
        [try block]
    catch [*ExceptionClass variable name]
        [catch block]
    end
    */
   // * ExceptionClass is optional, if not provided, it will catch all exceptions

    lavi::lang::parser::ast_node try_node(std::move(lexer.next_token()), ast_node_type::ast_node_try);

    static std::vector<std::string_view> valid_following_tokens = { "catch" };
    ast_node try_context = extract_context(lexer, *this, &valid_following_tokens);
    try_node.add_child(std::move(try_context));

    while(true) {
        const lavi::lang::lexer::token& possible_catch_token = lexer.see_next();

        if(possible_catch_token.type != lavi::lang::lexer::token_type::token_keyword || possible_catch_token.content != "catch") {
            break;
        }

        ast_node catch_node(std::move(lexer.next_token()), ast_node_type::ast_node_catch);

        const lavi::lang::lexer::token& possible_parenthesis_token = lexer.see_next();

        if(possible_parenthesis_token.type == lavi::lang::lexer::token_type::token_delimiter && possible_parenthesis_token.content == "(") {
            /*
                catch(ExceptionClass variable_name)
                     ^ '(' token
            */
            lexer.consume_token(); // Consume the '(' token

            const auto& possible_or_variable_name_class = lexer.see_next();
            /*
                catch(ExceptionClass variable_name)
                      ^^^^^^^^^^^^^^ Exception class
            */
            /*
                catch(variable_name)
                      ^^^^^^^^^^^^^ Variable
            */

            if(possible_or_variable_name_class.type != lavi::lang::lexer::token_type::token_identifier) {
                throw std::runtime_error(possible_or_variable_name_class.error_message_at_current_position("Expected exception class or variable name after 'catch'"));
            }

            const auto& possible_variable_name_token = lexer.see_next(1);
            /*
                catch(variable_name)
                      ^^^^^^^^^^^^^ Variable name
            */
            /*
                catch(ExceptionClass variable_name)
                                     ^^^^^^^^^^^^^ Variable name
            */
            if(possible_variable_name_token.type == lavi::lang::lexer::token_type::token_identifier)
            {
                /*
                  catch(ExceptionClass variable_name)
                                       ^^^^^^^^^^^^^ Variable name
                */
                catch_node.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_decltype));
                catch_node.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_declname));
            } else {
                /*
                    catch(variable_name)
                          ^^^^^^^^^^^^^ Variable name
                */
                catch_node.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_declname));
            }

            const auto& possible_closing_parenthesis = lexer.see_next();
            /*
                catch(ExceptionClass variable_name)
                                                  ^ ')' token
            */
            if(possible_closing_parenthesis.type != lavi::lang::lexer::token_type::token_delimiter || possible_closing_parenthesis.content != ")") {
                throw std::runtime_error(possible_closing_parenthesis.error_message_at_current_position("Expected closing ')' after exception class and variable name in 'catch'"));
            }

            lexer.consume_token(); // Consume the ')' token
        }

        ast_node catch_context = extract_context(lexer, *this, &valid_following_tokens);
        catch_node.add_child(std::move(catch_context));

        try_node.add_child(std::move(catch_node));
    }

    if(try_node.childrens().size() == 1) {
        throw std::runtime_error(try_node.token().error_message_at_current_position("Expected at least one 'catch' block after 'try'"));
    }

    try_node.set_end_token(lexer.next_token()); // Consume the closing 'end' token

    return try_node;
}

lavi::lang::parser::ast_node lavi::lang::parser::parse_keyword_enum(lavi::lang::lexer &lexer)
{
    ast_node enum_node(ast_node_type::ast_node_enum);
    enum_node.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_decltype));

    auto possible_name_token = lexer.see_next();

    if(possible_name_token.type != lavi::lang::lexer::token_type::token_identifier) {
        throw std::runtime_error(possible_name_token.error_message_at_current_position("Expected enum name after 'enum'"));
    }

    enum_node.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_declname));

    ast_node enum_values(ast_node_type::ast_node_arraydecl);

    while(true) {
        auto& possible_enum_entry = lexer.see_next();

        if(possible_enum_entry.type == lavi::lang::lexer::token_type::token_delimiter && possible_enum_entry.content == "end") {
            enum_node.set_end_token(lexer.next_token()); // Consume the 'end' token
            break;
        }

        if(possible_enum_entry.type != lavi::lang::lexer::token_type::token_identifier) {
            throw std::runtime_error(possible_enum_entry.error_message_at_current_position("Expected enum entry or 'end'"));
        }

        enum_values.add_child(ast_node(std::move(lexer.next_token()), ast_node_type::ast_node_valuedecl));
    }

    enum_node.add_child(std::move(enum_values));

    return enum_node;
}

lavi::lang::parser::exception::exception(std::string message, const lavi::lang::lexer::token& token)
    : m_token(token), m_message(std::move(message)), std::exception()
{
}

lavi::lang::parser::exception::exception(const lavi::lang::lexer::token& token)
    : m_token(token), std::exception()
{
}

lavi::lang::parser::unexpected_token_error::unexpected_token_error(const lavi::lang::lexer::token& token, std::string_view expected)
    : lavi::lang::parser::exception(token), m_expected(expected)
{
    m_message = generate_message();
}

std::string lavi::lang::parser::unexpected_token_error::generate_message() const
{
    std::string message;
    message.reserve(100);
    message = "Syntax error: unexpected " + token().human_description();
    if(!m_expected.empty()) {
        message += ", expected '";
        message += m_expected;
        message += "'";
    }
    message += " at ";
    message += token().human_start_position();

    return message;
}