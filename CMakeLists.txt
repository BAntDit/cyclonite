cmake_minimum_required(VERSION 3.10)

project(cyclonite VERSION 0.4.0.0)

option(ENABLE_SIMD_AVX2 "enable SIMD AVX2" OFF)
option(ENABLE_SIMD_AVX "enable SIMD AVX" OFF)
option(DISABLE_SIMD "force disable SIMD" OFF)

file(GLOB_RECURSE ALL_HEADERS "src/*.h")
set(PUBLIC_HEADERS ${ALL_HEADERS})
list(FILTER PUBLIC_HEADERS EXCLUDE REGEX ".*[\\\\/]internal[\\\\/].*\\.h$")

set(PRIVATE_HEADERS ${ALL_HEADERS})
list(REMOVE_ITEM PRIVATE_HEADERS ${PUBLIC_HEADERS})

file(GLOB_RECURSE SOURCES "src/*.cpp")

set(SHADER_SOURCES
    src/shaders/default.vert
    src/shaders/default.frag
    src/shaders/screen.vert
    src/shaders/screen.frag
)

add_library(${PROJECT_NAME} SHARED
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
    ${SOURCES}
)

find_program(GLSLANG_VALIDATOR glslangValidator)
message(STATUS "Found glslangValidator: ${GLSLANG_VALIDATOR}")

foreach(source ${SHADER_SOURCES})
    get_filename_component(dir "${source}" DIRECTORY)
    get_filename_component(name "${source}" NAME_WE)
    get_filename_component(ext "${source}" LAST_EXT)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}")
    set(source "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${dir}/${name}${ext}.spv.cpp.txt")

    add_custom_command(
        OUTPUT "${output}"
        COMMAND "${GLSLANG_VALIDATOR}" -V ${source} -x -o "${output}"
        DEPENDS "${source}"
    )

    target_sources(${PROJECT_NAME} PRIVATE "${output}")
endforeach()
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src")

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD ${REQUIRED_CXX_STANDARD}
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF

    COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"

    DEBUG_POSTFIX _d
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -pedantic
        -Wall
        -Wextra
        -Wfatal-errors
    )

    if (ENABLE_SIMD_AVX2)
        target_compile_options(${PROJECT_NAME} PRIVATE -mavx2)
        target_compile_definitions(${PROJECT_NAME} PUBLIC ENABLED_SIMD_AVX2)
        message("-- enable SIMD AVX2")
    elseif(ENABLE_SIMD_AVX)
        target_compile_options(${PROJECT_NAME} PRIVATE -mavx)
        target_compile_definitions(${PROJECT_NAME} PUBLIC ENABLED_SIMD_AVX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /Wall
    )

    if (ENABLE_SIMD_AVX2)
        target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
        target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLED_SIMD_AVX2)
    elseif(ENABLE_SIMD_AVX)
        target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX)
        target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLED_SIMD_AVX)
    endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_NAME="${PROJECT_NAME}"
    VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VERSION_PATCH=${PROJECT_VERSION_PATCH}
    VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
)

if (${VK_USE_PLATFORM_XLIB_KHR})
    message("-- platform: x11")
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_XLIB_KHR=1)
elseif(${VK_USE_PLATFORM_WAYLAND_KHR})
    message("-- platform: wayland")
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_WAYLAND_KHR=1)
elseif(${VK_USE_PLATFORM_XCB_KHR})
    message("-- platform: xcb")
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_XCB_KHR=1)
elseif(${VK_USE_PLATFORM_MIR_KHR})
    message("-- platform: mir")
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_MIR_KHR=1)
elseif(${VK_USE_PLATFORM_ANDROID_KHR})
    message("-- platform: android")
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_ANDROID_KHR=1)
elseif(${VK_USE_PLATFORM_WIN32_KHR})
    message("-- platform: windows")
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_WIN32_KHR=1)
endif()

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

find_package(TaskWeaver REQUIRED)
target_link_libraries(${PROJECT_NAME} TaskWeaver::TaskWeaver)

find_package(glm REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_RADIANS)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)

target_link_libraries(${PROJECT_NAME} glm::glm)
if (ENABLE_SIMD_AVX2)
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_AVX2)
elseif(ENABLE_SIMD_AVX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_AVX)
elseif(DISABLE_SIMD)
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_PURE)
endif()

find_package(SDL3 REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} SDL3::SDL3)

find_package(enttx REQUIRED)

find_package(metrix REQUIRED)

find_package(vulkan-validationlayers REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} vulkan-validationlayers::vulkan-validationlayers)

find_package(VulkanLoader REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} Vulkan::Loader)

find_package(Boost REQUIRED)
target_link_libraries(${PROJECT_NAME} boost::boost)

include(GNUInstallDirs)

# Install the library
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h"
)

# Install the export set
install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Create a config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

# Install the config files
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

set(ALL_SOURCES_FOR_STYLE_CORRECTION ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${SOURCES})
file(GLOB_RECURSE ALL_EXAMPLES_SOURCES_FOR_STYLE_CORRECTION "examples/*.cpp")
file(GLOB_RECURSE ALL_EXAMPLES_HEADERS_FOR_STYLE_CORRECTION "examples/*.h")

list(
    APPEND ALL_SOURCES_FOR_STYLE_CORRECTION
        ${ALL_EXAMPLES_SOURCES_FOR_STYLE_CORRECTION}
        ${ALL_EXAMPLES_HEADERS_FOR_STYLE_CORRECTION}
)

include(ClangFormat.cmake)
CODE_STYLE_CORRECTION("${ALL_SOURCES_FOR_STYLE_CORRECTION}")

include(SubDirectoryList.cmake)

SUBDIRLIST(EXANPLEDIRS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
foreach(example ${EXANPLEDIRS})
  add_subdirectory("examples/${example}")
endforeach()

option(TESTS "whether needs to build tests" ON)

if (TESTS)
    find_package(GTest REQUIRED)

   enable_testing()

    add_subdirectory("tests")
endif()
