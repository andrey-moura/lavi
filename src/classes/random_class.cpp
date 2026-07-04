#include <lavi/lang/api.hpp>
#include <lavi/lang/classes.hpp>
#include <random>
#include <climits>

void create_random_class()
{
    lavi::lang::random_class = lavi::lang::klass::create_builtin("Random");
    lavi::lang::random_class->functions["integer"] = std::make_shared<lavi::lang::function>("integer", [](lavi::lang::interpreter* interpreter) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(INT_MIN, INT_MAX);
        return lavi::lang::object::instantiate(interpreter, lavi::lang::integer_class, dis(gen));
    });
}