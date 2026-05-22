#pragma once

#include <filesystem>

namespace lavi
{
    namespace lang
    {
        namespace config
        {
            std::filesystem::path src_dir();
            const std::string_view version = LAVI_VERSION;
            const std::string_view build = ANDY_BUILD_TYPE;
            
            const std::string_view cpp      = ANDY_CPP_VERSION;
            const std::string_view compiler = ANDY_COMPILER;
        };
    };
}