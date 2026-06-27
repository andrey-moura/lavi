#pragma once

#include <map>
#include <vector>
#include <string_view>
#include <memory>
#include <deque>

namespace lavi
{
  namespace lang
  {
    class object;
    class function;
    using inline_function = std::shared_ptr<lavi::lang::object>(*)(lavi::lang::interpreter*, std::shared_ptr<lavi::lang::object>&, const lavi::lang::parser::ast_node&);
    class klass;
    struct scope
    {
      public:
        std::map<std::string, std::shared_ptr<lavi::lang::object>, std::less<>> variables;
        std::map<std::string, std::shared_ptr<lavi::lang::function>, std::less<>> functions;
        std::map<std::string, std::shared_ptr<lavi::lang::inline_function>, std::less<>> inline_functions;
    };
  }
};