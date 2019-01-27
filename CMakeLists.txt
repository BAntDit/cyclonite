cmake_minimum_required(VERSION 3.10)

project(cyclonite VERSION 0.0.0.0)

set(PUBLIC_HEADERS
    src/options.h
    src/app.h
    src/core/typedefs.h
    src/core/transform.h
    src/core/sceneManager.h
    src/core/scene.h
    src/multithreading/taskManager.h
    src/loaders/glTF/loader.h
)

set(PRIVATE_HEADERS
    src/loaders/glTF/internal/getOptional.h
    src/loaders/glTF/internal/readArray.h
)

set(SOURCES
    src/options.cpp
    src/multithreading/taskManager.cpp
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

set(ALL_SOURCES_FOR_STYLE_CORRECTION ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${SOURCES})

include(ClangFormat.cmake)
CODE_STYLE_CORRECTION("${ALL_SOURCES_FOR_STYLE_CORRECTION}")
