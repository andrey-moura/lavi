#pragma once

#include <memory>
#include <string>

namespace lavi
{
    namespace lang
    {
        class interpreter;
        class klass;
        class object;
        // This exception is thrown when an error occurs in the andylang interpreter.
        class exception : public std::exception
        {
            public:
                exception(
                    lavi::lang::interpreter* __interpreter,
                    std::string what,
                    std::shared_ptr<lavi::lang::klass> __klass = nullptr
                );
                exception(
                    lavi::lang::interpreter* __interpreter,
                    std::shared_ptr<lavi::lang::object> __exception_object
                );
            public:
                std::shared_ptr<lavi::lang::object> exception_object;
                lavi::lang::interpreter* interpreter;
                std::string message;
            public:
                const char* what() const noexcept override;
        };
    }; // namespace lang
};

