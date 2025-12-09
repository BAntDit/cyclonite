file(GLOB TOOLS_SHADER_COMPILER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library("${PROJECT_NAME}-lib" ${TOOLS_SHADER_COMPILER_HEADERS} ${TOOLS_SHADER_COMPILER_SOURCES})

set_target_properties("${PROJECT_NAME}-lib" PROPERTIES
        VERSION ${PROJECT_VERSION}

        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS OFF

        DEBUG_POSTFIX _d
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options("${PROJECT_NAME}-lib" PRIVATE
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

target_include_directories("${PROJECT_NAME}-lib" PUBLIC "${TOOL_COMMON_INCLUDE_DIRS}")

find_package(dxcompiler REQUIRED)
target_link_libraries("${PROJECT_NAME}-lib" PRIVATE dxcompiler::dxcompiler)

