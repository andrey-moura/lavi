#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

file(READ "${CMAKE_SOURCE_DIR}/VERSION" LAVILANG_PROJECT_VERSION)
string(STRIP "${LAVILANG_PROJECT_VERSION}" LAVILANG_PROJECT_VERSION)
set(LAVILANG_VERSION "${LAVILANG_PROJECT_VERSION}")

string(FIND "${LAVILANG_VERSION}" "-beta" IS_BETA)
if(NOT IS_BETA EQUAL -1)
    # Removes the -beta* part
    string(SUBSTRING "${LAVILANG_VERSION}" 0 ${IS_BETA} LAVILANG_VERSION)
endif()

string(FIND "${LAVILANG_VERSION}" "-alpha" IS_ALPHA)
if(NOT IS_ALPHA EQUAL -1)
    # Removes the -alpha* part
    string(SUBSTRING "${LAVILANG_VERSION}" 0 ${IS_ALPHA} LAVILANG_VERSION)
endif()

project(lavi VERSION ${LAVILANG_VERSION})

option(BUILD_LAVI_CPP "Build lavic++" OFF)
option(BUILD_LAVI_ANALYZER "Build lavi-analyzer" ON)

# Get the parent directory
get_filename_component(LAVILANG_PARENT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

include(${CMAKE_CURRENT_LIST_DIR}/modules/andy-tests/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/modules/andy/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/modules/andy-console/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/modules/andy-file/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/modules/andy-core/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/modules/binary/CMakeLists.txt)

include("${CMAKE_CURRENT_LIST_DIR}/src/builtin_libs/net_extension/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/src/builtin_libs/yaml_extension/CMakeLists.txt")

add_library(lavi-lang
    ${CMAKE_CURRENT_LIST_DIR}/src/api.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/function.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/preprocessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/extension.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/error.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/interpreter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/object.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/lexer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/parser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/builtin_libs.cpp
    
    ${CMAKE_CURRENT_LIST_DIR}/src/yaml.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/hash.cpp
    
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/std_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/andy_config_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/array_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/class_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/hash_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/directory_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/double_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/false_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/file_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/float_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/function_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/integer_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/null_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/path_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/string_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/system_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/true_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/classes/exception_class.cpp

    ${CMAKE_CURRENT_LIST_DIR}/modules/andy-tests/src/andy-tests.cpp
    
    ${LAVI_YAML_EXTENSION_SOURCES}
    ${ANDY_NET_EXTENSION_SOURCES}

    $<TARGET_OBJECTS:andy-file>
    $<TARGET_OBJECTS:andy-core>
    $<TARGET_OBJECTS:andy-console>
    $<TARGET_OBJECTS:andy-binary>
)

target_link_libraries(lavi-lang PRIVATE ${ANDY_NET_LIBRARIES} ${LAVI_DRAWING_BACKEND_LIBRARIES})

add_executable(lavi
    ${CMAKE_CURRENT_LIST_DIR}/src/lavi.cpp
)

if(FALSE)
    find_package(Clang REQUIRED)

    include_directories(${CLANG_INCLUDE_DIRS})

    add_executable(lavic++
        ${CMAKE_CURRENT_LIST_DIR}/src/lavic++.cpp
    )

    target_link_libraries(lavic++ PRIVATE lavi-lang libclang)
endif()

if(BUILD_LAVI_ANALYZER)
    add_executable(lavi-analyzer
        ${CMAKE_CURRENT_LIST_DIR}/src/analyzer.cpp
    )

    target_link_libraries(lavi-analyzer PRIVATE lavi-lang)
endif()

if(UNIX)
    find_package(Threads REQUIRED)

    target_link_libraries(lavi-lang PRIVATE ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
    target_link_options(lavi PRIVATE -Wl,--export-dynamic)

    # set_target_properties(lavi PROPERTIES LINK_SEARCH_START_STATIC ON)
    # set_target_properties(lavi PROPERTIES LINK_SEARCH_END_STATIC ON)
    # target_link_options(lavi PRIVATE -static-libgcc -static-libstdc++ -static)
elseif(WIN32)
    add_executable(lavi-installer WIN32 ${CMAKE_CURRENT_LIST_DIR}/win32/installer.cpp)
    if(EXISTS "${CMAKE_BINARY_DIR}/Release/lavi.exe")
        enable_language("RC")
        set(WIN32_RESOURCES ${CMAKE_CURRENT_LIST_DIR}/win32/resources.rc)
        target_sources(lavi PRIVATE ${WIN32_RESOURCES})
        target_sources(lavi-installer PRIVATE ${WIN32_RESOURCES})
    endif()

    add_dependencies(lavi-installer lavi)
    if(MSVC)
        SET_TARGET_PROPERTIES(lavi-installer PROPERTIES LINK_FLAGS "/MANIFESTUAC:/level='requireAdministrator' /uiAccess='false'")
    endif()
    target_compile_definitions(lavi-lang PRIVATE _WIN32_WINNT=0x0A00)
endif()

target_link_libraries(lavi PRIVATE lavi-lang)

set(LAVILANG_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(LAVILANG_BUILD_DIR "${CMAKE_BINARY_DIR}")

if(WIN32)
    # Replace / with \\
    string(REPLACE "/" "\\\\" LAVILANG_PROJECT_DIR "${LAVILANG_PROJECT_DIR}")
    string(REPLACE "/" "\\\\" LAVILANG_BUILD_DIR "${LAVILANG_BUILD_DIR}")
endif()

add_definitions(-DLAVI_PROJECT_DIR="${CMAKE_CURRENT_LIST_DIR}")
add_definitions(-DLAVI_BUILD_DIR="${CMAKE_BINARY_DIR}")
add_definitions(-DLAVI_VERSION="${LAVILANG_PROJECT_VERSION}")

# Install lavi executable
install(TARGETS lavi DESTINATION bin)
if(BUILD_LAVI_ANALYZER)
    # Install lavi-analyzer executable
    install(TARGETS lavi-analyzer DESTINATION bin)
endif()
# Install lavi-lang library
install(TARGETS lavi-lang DESTINATION lib)
# Install lavi-lang headers
install(DIRECTORY include/lavi DESTINATION include)
install(DIRECTORY std DESTINATION include/lavi)
install(DIRECTORY utility DESTINATION src/lavi)

install(EXPORT lavi-langTargets
    FILE lavi-langTargets.cmake
    NAMESPACE lavi-lang::
    DESTINATION lib/cmake/lavi-lang
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/lavi-langConfigVersion.cmake
    VERSION ${LAVILANG_PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/lavi-langConfig.cmake.in
    ${CMAKE_BINARY_DIR}/lavi-langConfig.cmake
    @ONLY
)

install(FILES
    ${CMAKE_BINARY_DIR}/lavi-langConfig.cmake
    ${CMAKE_BINARY_DIR}/lavi-langConfigVersion.cmake
    DESTINATION lib/cmake/lavi-lang
)

install(TARGETS lavi-lang
    EXPORT lavi-langTargets
)

andy_tests_configure(lavi)
andy_tests_configure(lavi-lang)