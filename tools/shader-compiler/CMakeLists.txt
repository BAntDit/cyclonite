
cmake_minimum_required(VERSION 3.17.2)
project(tools.shader-compiler VERSION 0.0.1.0)

file(GLOB TOOLS_SHADER_COMPILER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h")

set(TOOL_COMMON_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/includes/")

add_subdirectory("lib")
add_subdirectory("app")