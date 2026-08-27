
file(GLOB TOOLS_ASSET_TOOL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library("${PROJECT_NAME}-lib"
        ${ALL_SHARED_HEADERS}
        ${TOOLS_ASSET_TOOL_HEADERS}
        ${TOOLS_ASSET_TOOL_SOURCES})

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
    target_compile_options("${PROJECT_NAME}-lib" PRIVATE
            /Wall
    )
endif()

target_include_directories("${PROJECT_NAME}-lib" PRIVATE $<BUILD_INTERFACE:${SHARED_HEADERS_DIR}>)
target_include_directories("${PROJECT_NAME}-lib" PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/shared/>)
target_include_directories("${PROJECT_NAME}-lib" PUBLIC $<BUILD_INTERFACE:${TOOL_COMMON_INCLUDE_DIRS}>)
