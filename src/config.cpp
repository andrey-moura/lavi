#include <lavi/lang/config.hpp>

namespace lavi
{
    namespace lang
    {
        namespace config
        {
            std::filesystem::path src_dir()
            {
                std::filesystem::path src_dir;
#ifdef __ANDY_DEBUG__
                return std::filesystem::absolute(LAVI_PROJECT_DIR);
#endif
#ifdef __linux__
                src_dir = std::filesystem::path("/usr/local/src/lavi");
#elif defined(__wasm__)
                src_dir = std::filesystem::path("/");
#elif defined(_WIN32)
                src_dir = std::filesystem::path("C:/Program Files (x86)/lavi/src/lavi");
#else
                throw std::runtime_error("unsupported OS");
#endif
                return src_dir;
            }
        };
    };
};
