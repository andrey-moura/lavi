

include(CMakeFindDependencyMacro)

set(andy-lang_INCLUDE_DIRS "/usr/local/include")
set(andy-lang_LIBRARIES andy-lang)

include("${CMAKE_CURRENT_LIST_DIR}/andy-langTargets.cmake")

add_compile_definitions(
    ANDY_VERSION="${ANDY_VERSION}"
    ANDY_BUILD_TYPE="${ANDY_BUILD_TYPE}"
    ANDY_CPP_VERSION="${ANDY_CPP_VERSION}"
    ANDY_COMPILER="${ANDY_COMPILER}"
    LAVI_VERSION="${LAVI_PROJECT_VERSION}"
)
