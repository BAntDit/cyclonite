cmake_minimum_required(VERSION 3.10)

project(basic.example VERSION 0.0.1.0)

file(GLOB BASIC_EXAMPLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB BASIC_EXAMPLE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(${PROJECT_NAME} ${BASIC_EXAMPLE_HEADERS} ${BASIC_EXAMPLE_SOURCES})
add_dependencies(${PROJECT_NAME} cyclonite)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}

    CXX_STANDARD ${REQUIRED_CXX_STANDARD}
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
        /W4
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/")
message(STATUS "cyclonite sources dir = ${CMAKE_SOURCE_DIR}/src/")

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/shared/")
message(STATUS "cyclonite shared sources dir = ${CMAKE_SOURCE_DIR}/shared/")

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}")
endif()

message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

get_target_property(CYCLONITE_ARCHIVE_DIR_RELEASE cyclonite ARCHIVE_OUTPUT_DIRECTORY_RELEASE)
get_target_property(CYCLONITE_ARCHIVE_DIR_DEBUG cyclonite ARCHIVE_OUTPUT_DIRECTORY_DEBUG)
message(STATUS "cyclonite ARCHIVE_OUTPUT_DIRECTORY_RELEASE = ${CYCLONITE_ARCHIVE_DIR_RELEASE}")
message(STATUS "cyclonite ARCHIVE_OUTPUT_DIRECTORY_DEBUG = ${CYCLONITE_ARCHIVE_DIR_DEBUG}")

target_link_libraries(${PROJECT_NAME} PRIVATE cyclonite)

target_link_libraries(${PROJECT_NAME} PRIVATE vulkan-validationlayers::vulkan-validationlayers)

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Loader)

target_link_libraries(${PROJECT_NAME} PRIVATE boost::boost)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

find_package(metrix REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE metrix::metrix)

#target_link_libraries(${PROJECT_NAME} enttx::enttx)