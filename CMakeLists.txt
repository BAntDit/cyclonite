cmake_minimum_required(VERSION 3.10)

project(cyclonite VERSION 0.0.0.0)

set(PUBLIC_HEADERS
    src/config.h
    src/options.h
    src/app.h
    src/updateStages.h
    src/rangeBasedMemoryManager.h
    src/core/typedefs.h
    src/core/bufferView.h
    src/core/transform.h
    src/core/camera.h
    src/core/sceneManager.h
    src/core/scene.h
    src/core/attributeSemantic.h
    src/core/primitiveType.h
    src/core/vertexAttribute.h
    src/core/geometry.h
    src/vulkan/handle.h
    src/vulkan/instance.h
    src/vulkan/memoryPage.h
    src/multithreading/taskManager.h
    src/loaders/glTF/loader.h
)

set(PRIVATE_HEADERS
    src/loaders/glTF/internal/getOptional.h
    src/loaders/glTF/internal/readArray.h
    src/loaders/glTF/internal/base64Decode.h
)

set(SOURCES
    src/options.cpp
    src/vulkan/instance.cpp
    src/multithreading/taskManager.cpp
    src/loaders/glTF/internal/base64Decode.cpp
    src/loaders/glTF/loader.cpp
)

add_library(${PROJECT_NAME} SHARED
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
    ${SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
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
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /Wall
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_NAME="${PROJECT_NAME}"
    VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VERSION_PATCH=${PROJECT_VERSION_PATCH}
    VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
)

find_package(glm REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_RADIANS)
target_link_libraries(${PROJECT_NAME} glm)

find_package(enttx REQUIRED)

find_package(nlohmann_json REQUIRED)

find_package(Boost REQUIRED COMPONENTS
    system
    program_options
)

target_link_libraries(${PROJECT_NAME}
    Boost::boost
    Boost::system
    Boost::program_options
)

find_package(Vulkan REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_XLIB_KHR)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

set(ALL_SOURCES_FOR_STYLE_CORRECTION ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${SOURCES})

include(ClangFormat.cmake)
CODE_STYLE_CORRECTION("${ALL_SOURCES_FOR_STYLE_CORRECTION}")
