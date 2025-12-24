
cmake_minimum_required(VERSION 3.17.2)
project(tools.shader-compiler VERSION 0.0.1.0)

file(GLOB TOOLS_SHADER_COMPILER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h")

set(TOOL_COMMON_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/includes/")

file(GLOB_RECURSE ALL_SHARED_HEADERS "../shared/*.h")
file(GLOB_RECURSE ALL_HEADERS "includes/*.h")
file(GLOB_RECURSE ALL_LIB_SOURCES "lib/*.cpp")
file(GLOB_RECURSE ALL_APP_SOURCES "app/*.cpp")

set(ALL_SOURCES_FOR_STYLE_CORRECTION ${ALL_HEADERS} ${ALL_LIB_SOURCES} ${ALL_APP_SOURCES} ${ALL_SHARED_HEADERS})

include("${CMAKE_CURRENT_SOURCE_DIR}/../../ClangFormat.cmake")
CODE_STYLE_CORRECTION("${ALL_SOURCES_FOR_STYLE_CORRECTION}")

add_subdirectory("lib")
add_subdirectory("app")
