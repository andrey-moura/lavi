#include <lavi/lang/function.hpp>
#include <lavi/lang/object.hpp>

namespace lavi {
    namespace lang {
        function_call::function_call(
            std::string_view __name,
            std::shared_ptr<lavi::lang::object> __object,
            std::vector<std::shared_ptr<lavi::lang::object>> __positional_params,
            std::map<std::string_view, std::shared_ptr<lavi::lang::object>> __named_params
        ) : name(std::move(__name)), object(std::move(__object)), cls(__object ? __object->cls : nullptr), positional_params(std::move(__positional_params)), named_params(std::move(__named_params))
        {

        }

        function_call::function_call(
            std::string_view __name,
            std::shared_ptr<lavi::lang::structure> __cls,
            std::shared_ptr<lavi::lang::object> __object,
            const lavi::lang::function* method,
            std::vector<std::shared_ptr<lavi::lang::object>> __positional_params,
            std::map<std::string_view, std::shared_ptr<lavi::lang::object>> __named_params,
            const lavi::lang::parser::ast_node* __given_block
        ) : name(std::move(__name)), cls(std::move(__cls)), object(std::move(__object)), method(method), positional_params(std::move(__positional_params)), named_params(std::move(__named_params)), given_block(__given_block)
        {

        }
    };
};