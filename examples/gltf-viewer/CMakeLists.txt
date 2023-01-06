cmake_minimum_required(VERSION 3.10)

project(gltf-viewer.example VERSION 0.0.0.0)

set(HEADER_FILES
    model.h
    view.h
    controller.h
    gltf/reader.h
    viewer.h
    appConfig.h
)

set(SOURCE_FILES
    gltf/reader.cpp
    model.cpp
    view.cpp
    controller.cpp
    viewer.cpp
)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}

    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF

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

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../../src/")

target_link_libraries(${PROJECT_NAME} cyclonite)

target_link_libraries(${PROJECT_NAME} Boost::boost)

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${PROJECT_NAME} glm)

target_link_libraries(${PROJECT_NAME} SDL2)

find_package(nlohmann_json REQUIRED)
