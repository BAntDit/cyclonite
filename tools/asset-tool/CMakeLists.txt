
cmake_minimum_required(VERSION 3.17.2)
project(asset-tool VERSION 0.0.1.0)

file(GLOB TOOLS_ASSET_TOOL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h")

set(TOOL_COMMON_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/includes/")

file(GLOB_RECURSE ALL_SHARED_HEADERS "../shared/*.h")
file(GLOB_RECURSE ALL_HEADERS "includes/*.h")
file(GLOB_RECURSE ALL_LIB_SOURCES "lib/*.cpp")
file(GLOB_RECURSE ALL_CONSOLE_SOURCES "console/*.cpp")

# Include standard installation directories
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT DEFINED SHARED_HEADERS_DIR)
    set(SHARED_HEADERS_DIR "${CMAKE_SOURCE_DIR}/../../shared")
endif()

add_subdirectory("lib")
add_subdirectory("console")
