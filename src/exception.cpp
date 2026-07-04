#include "lavi/lang/exception.hpp"
#include "lavi/lang/lang.hpp"
#include "lavi/lang/interpreter.hpp"
#include "lavi/lang/api.hpp"

namespace lavi
{
    namespace lang
    {
        exception::exception(
            lavi::lang::interpreter* __interpreter,
            std::string what,
            std::shared_ptr<lavi::lang::klass> __klass
        )
            : interpreter(__interpreter), message(std::move(what))
        {
            if(!__klass) {
                __klass = lavi::lang::runtime_error_class;
            }
            exception_object = lavi::lang::api::new_object(
                interpreter,
                __klass,
                { 
                    lavi::lang::api::to_object(interpreter, message)
                }
            );
        }
        exception::exception(
            lavi::lang::interpreter* __interpreter,
            std::shared_ptr<lavi::lang::object> __exception_object
        )
            : interpreter(__interpreter), exception_object(std::move(__exception_object))
        {
            auto message = lavi::lang::api::call(interpreter, "message", exception_object);
            if(message) {
                this->message = message->as<std::string>();
            } else {
                this->message = "Unknown error";
            }
        }
        const char* exception::what() const noexcept
        {
            return message.c_str();
        }
    }; // namespace lang
}; // namespace lavi