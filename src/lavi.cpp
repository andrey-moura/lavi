#include <iostream>
#include <filesystem>

#include <lavi/lang/api.hpp>
#include <lavi/lang/lang.hpp>
#include <lavi/lang/exception.hpp>
#include <andy/console.hpp>

std::shared_ptr<lavi::lang::interpreter> interpreter;

int main(int argc, char** argv) {
    try {
        std::filesystem::path andy_executable_path = argv[0];
        //vm_instance = std::make_shared<lavi::lang::vm>();

        std::filesystem::path file_path;

        if(argc > 1) {
            std::string_view arg = argv[1];

            if(arg.starts_with("--")) {
                if(arg == "--help") {
                    std::cout << "Usage: " << argv[0] << " [file]" << std::endl;
                    std::cout << std::endl;
                    std::cout << "Options: " << std::endl;
                    lavi::console::print_warning("  --help");
                    std::cout << "     Display this information" << std::endl;
                    lavi::console::print_warning("  --version");
                    std::cout << "  Display the version of the andy language" << std::endl;
                    return 0;
                } else if(arg == "--version") {
                    std::cout << LAVI_VERSION << std::endl;
                    return 0;
                }
            }
        }

        if(file_path.empty()) {
            if(argc > 1) {
                file_path = std::filesystem::absolute(argv[1]);
            } else {
                file_path = std::filesystem::absolute("application.lv");
            }

            if(file_path.extension() != ".lv") {
                file_path.replace_extension(".lv");
            }

            if(!std::filesystem::exists(file_path)) {
                if(!std::filesystem::exists(file_path)) {
                    file_path = lavi::lang::config::src_dir() / "utility" / argv[1];
                    file_path.replace_extension(".lv");
                }
                if(!std::filesystem::exists(file_path)) {
                    throw std::runtime_error("input file does not exist");
                }
            }

            if(!std::filesystem::is_regular_file(file_path)) {
                if(file_path.extension() != ".lv") {
                    file_path.replace_extension(".lv");
                }
                if(!std::filesystem::is_regular_file(file_path)) {
                    throw std::runtime_error("input file is not a regular file");
                }
            }
        }

        interpreter = std::make_shared<lavi::lang::interpreter>();

        std::shared_ptr<lavi::lang::object> ret = lavi::lang::api::evaluate(interpreter.get(), file_path, argc, argv);

        if(!ret) {
            return 0;
        }

        if(ret) {
            // TODO: Treat the return value
            int ret_value = ret->as<int>();
            return ret_value;
        }
    } catch (const lavi::lang::exception& e) {
        std::cerr << e.exception_object->klass->name << ": " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}