
cmake_minimum_required(VERSION 3.17.2)
project(shader-compiler VERSION 0.1.2.0)

file(GLOB TOOLS_SHADER_COMPILER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h")

set(TOOL_COMMON_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/includes/")

file(GLOB_RECURSE ALL_SHARED_HEADERS "../shared/*.h")
file(GLOB_RECURSE ALL_HEADERS "includes/*.h")
file(GLOB_RECURSE ALL_LIB_SOURCES "lib/*.cpp")
file(GLOB_RECURSE ALL_APP_SOURCES "app/*.cpp")

# Include standard installation directories
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT DEFINED SHARED_HEADERS_DIR)
    set(SHARED_HEADERS_DIR "${CMAKE_SOURCE_DIR}/../../shared")
endif()

add_subdirectory("lib")
add_subdirectory("app")

# Generate package configuration files
set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/shader-compiler)

# Install headers
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/shared/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/shared
    FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${SHARED_HEADERS_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/shared
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Create package version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/shader-compiler-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION ${CONFIG_INSTALL_DIR}
)

