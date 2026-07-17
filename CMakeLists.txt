cmake_minimum_required(VERSION 3.10)

project(cyclonite VERSION 0.5.0.0)

option(ENABLE_SIMD_AVX2 "enable SIMD AVX2" OFF)
option(ENABLE_SIMD_AVX "enable SIMD AVX" OFF)
option(DISABLE_SIMD "force disable SIMD" OFF)

file(GLOB_RECURSE ALL_HEADERS "src/*.h")
set(PUBLIC_HEADERS ${ALL_HEADERS})
list(FILTER PUBLIC_HEADERS EXCLUDE REGEX ".*[\\\\/]internal[\\\\/].*\\.h$")

set(PRIVATE_HEADERS ${ALL_HEADERS})
list(REMOVE_ITEM PRIVATE_HEADERS ${PUBLIC_HEADERS})

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE INL_FILES "src/*.inl")

file(GLOB_RECURSE ALL_SHARED_HEADERS "shared/*.h")

add_library(${PROJECT_NAME} SHARED
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
    ${SOURCES}
)

set(GAPI_SHADER_COMPILATION_ARGS)
if (${GAPI_VULKAN})
    set(GAPI_SHADER_COMPILATION_ARGS "--target-gapi vulkan --spirv --fvk-use-gl-layout --fspv-target-env vulkan1.2")
elseif(${GAPI_D3D12})
    set(GAPI_SHADER_COMPILATION_ARGS "--target-gapi d3d12")
endif()

set(COMMON_SHADER_COMPILATION_ARGS "--target-platform all --HV 2021 --O3 ${GAPI_SHADER_COMPILATION_ARGS}")

set(SHADER_SOURCES
    "src/shaders/default.fs.hlsl --E main --T ps_6_9 ${COMMON_SHADER_COMPILATION_ARGS}"
    "src/shaders/default.vs.hlsl --E main --T vs_6_9 ${COMMON_SHADER_COMPILATION_ARGS}"
    "src/shaders/screen.fs.hlsl --E main --T ps_6_9 ${COMMON_SHADER_COMPILATION_ARGS}"
    "src/shaders/screen.vs.hlsl --E main --T vs_6_9 ${COMMON_SHADER_COMPILATION_ARGS}"
    "src/shaders/testTriangle.vs.hlsl --E main --T vs_6_9 ${COMMON_SHADER_COMPILATION_ARGS}"
    "src/shaders/testTriangle.fs.hlsl --E main --T ps_6_9 ${COMMON_SHADER_COMPILATION_ARGS}"
    "src/shaders/testBox.vs.hlsl --E main --T vs_6_9 ${COMMON_SHADER_COMPILATION_ARGS}"
    "src/shaders/testBox.fs.hlsl --E main --T ps_6_9 ${COMMON_SHADER_COMPILATION_ARGS}"
)

find_package(shader-compiler REQUIRED)

foreach(source ${SHADER_SOURCES})
    string(REPLACE " " ";" COMPILER_ARGUMENTS ${source})
    list(GET COMPILER_ARGUMENTS 0 FILE_PATH)
    list(SUBLIST COMPILER_ARGUMENTS 1 -1 REST_ARGS)
    get_filename_component(dir "${FILE_PATH}" DIRECTORY)
    get_filename_component(name "${FILE_PATH}" NAME_WE)
    get_filename_component(ext "${FILE_PATH}" EXT)
    get_filename_component(first_ext "${ext}" NAME_WE)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}")
    set(source "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}")
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${dir}/${name}${ext}.sm.bin")

    message(STATUS "Adding custom command for: ${source} -> ${output}")

    add_custom_command(
        OUTPUT "${output}"
        COMMAND shader-compiler::shader-compiler-app --source ${source} ${REST_ARGS} --Fo "${output}"
        DEPENDS "${source}"
    )

    target_sources(${PROJECT_NAME} PRIVATE "${output}")
endforeach()

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

if (${PLATFORM_LINUX_X11})
    message("-- platform: x11")
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_LINUX_X11=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_XLIB_KHR=1)
elseif(${PLATFORM_LINUX_WAYLAND})
    message("-- platform: wayland")
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_LINUX_WAYLAND=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_WAYLAND_KHR=1)
elseif(${PLATFORM_LINUX_XCB})
    message("-- platform: xcb")
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_LINUX_XCB=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_XCB_KHR=1)
elseif(${PLATFORM_LINUX_MIR})
    message("-- platform: mir")
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_LINUX_MIR=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_MIR_KHR=1)
elseif(${PLATFORM_ANDROID})
    message("-- platform: android")
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_ANDROID=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_ANDROID_KHR=1)
elseif(${PLATFORM_WINDOWS})
    message("-- platform: windows")
    add_compile_definitions(NOMINMAX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_WINDOWS=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_WIN32_KHR=1)
endif()

if (${GAPI_VULKAN})
    message("-- gapi: vulkan")
    target_compile_definitions(${PROJECT_NAME} PUBLIC GFX_DRIVER_VULKAN=1)
elseif(${GAPI_D3D12})
    message("-- gapi: d3d12")
    target_compile_definitions(${PROJECT_NAME} PUBLIC GFX_DRIVER_D3D12=1)
endif()

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

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
target_link_libraries(${PROJECT_NAME} enttx::enttx)

find_package(metrix REQUIRED)

find_package(vulkan-validationlayers REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} vulkan-validationlayers::vulkan-validationlayers)

find_package(VulkanLoader REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} Vulkan::Loader)

find_package(Boost REQUIRED)
target_link_libraries(${PROJECT_NAME} boost::boost)

find_package(vulkan-memory-allocator REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} vulkan-memory-allocator::vulkan-memory-allocator)

if(NOT DEFINED SHARED_HEADERS_DIR)
    set(SHARED_HEADERS_DIR "${CMAKE_SOURCE_DIR}/../../shared")
endif()

target_include_directories("${PROJECT_NAME}" PRIVATE $<BUILD_INTERFACE:${SHARED_HEADERS_DIR}>)
target_include_directories("${PROJECT_NAME}" PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/shared/>)

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

option(TESTS "whether needs to build tests" ON)

set(ALL_SOURCES_FOR_STYLE_CORRECTION ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${SOURCES} ${INL_FILES} ${ALL_SHARED_HEADERS})
file(GLOB_RECURSE ALL_EXAMPLES_SOURCES_FOR_STYLE_CORRECTION "examples/*.cpp")
file(GLOB_RECURSE ALL_EXAMPLES_HEADERS_FOR_STYLE_CORRECTION "examples/*.h")

list(
    APPEND ALL_SOURCES_FOR_STYLE_CORRECTION
        ${ALL_EXAMPLES_SOURCES_FOR_STYLE_CORRECTION}
        ${ALL_EXAMPLES_HEADERS_FOR_STYLE_CORRECTION}
)

if (TESTS)
    file(GLOB_RECURSE ALL_TESTS_SOURCES_FOR_STYLE_CORRECTION "tests/*.cpp")
    file(GLOB_RECURSE ALL_TESTS_HEADERS_FOR_STYLE_CORRECTION "tests/*.h")

    list(
    APPEND ALL_SOURCES_FOR_STYLE_CORRECTION
        ${ALL_TESTS_SOURCES_FOR_STYLE_CORRECTION}
        ${ALL_TESTS_HEADERS_FOR_STYLE_CORRECTION}
    )
endif()

include(ClangFormat.cmake)
CODE_STYLE_CORRECTION("${ALL_SOURCES_FOR_STYLE_CORRECTION}")

include(SubDirectoryList.cmake)
SUBDIRLIST(EXANPLEDIRS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
foreach(example ${EXANPLEDIRS})
  add_subdirectory("examples/${example}")
endforeach()

if (TESTS)
    find_package(GTest REQUIRED)

    enable_testing()

    add_subdirectory("tests")
endif()
