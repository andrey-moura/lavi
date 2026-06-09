#include "lavi/lang/object.hpp"
#include "lavi/lang/classes.hpp"
#include "lavi/lang/api.hpp"

void create_class_class()
{
    lavi::lang::class_class = lavi::lang::klass::create_builtin("Class");

    lavi::lang::class_class->instance_functions["init"] = std::make_shared<lavi::lang::function>("init", std::initializer_list<std::string>{ "class_name" }, [](lavi::lang::interpreter* interpreter) {
        auto object = interpreter->current_context->self;
        auto& params = interpreter->current_context->positional_params;
        if(params.size() == 1) {
            const std::string& class_name = params[0]->as<std::string>();

            auto cls = interpreter->find_class(class_name);

            if(!cls) {
                throw std::runtime_error("class " + class_name + " not found");
            }

            object->variables["name"] = params[0];
            object->set_native<std::shared_ptr<lavi::lang::klass>>(cls);
        } else {
            // Called from interpreter
            object->variables["name"] = lavi::lang::api::to_object(interpreter, object->as<std::shared_ptr<lavi::lang::klass>>()->name);
        }
        return nullptr;
    });

}