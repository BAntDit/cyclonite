cmake_minimum_required(VERSION 3.10)

project("cyclonite.test")

set(SOURCES
    resourceManagementTest.cpp
    resourceManagementTests.h
    taskManagerTest.cpp
    taskManagerTest.h)

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
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
            /Wall
            )
endif()

target_link_libraries(${PROJECT_NAME} gtest gtest_main -pthread)

add_test(NAME cyclonite-test COMMAND ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../../src/")

target_link_libraries(${PROJECT_NAME} cyclonite)

target_link_libraries(${PROJECT_NAME} vulkan-validationlayers::vulkan-validationlayers)

target_link_libraries(${PROJECT_NAME} Vulkan::Loader)

target_link_libraries(${PROJECT_NAME} boost::boost)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${PROJECT_NAME} glm::glm)

target_link_libraries(${PROJECT_NAME} SDL3::SDL3)