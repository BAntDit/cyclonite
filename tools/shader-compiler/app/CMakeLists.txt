
file(GLOB TOOLS_SHADER_COMPILER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable("${PROJECT_NAME}-app" ${TOOLS_SHADER_COMPILER_HEADERS} ${TOOLS_SHADER_COMPILER_SOURCES})

set_target_properties("${PROJECT_NAME}-app" PROPERTIES
    VERSION ${PROJECT_VERSION}

    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF

    DEBUG_POSTFIX _d
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options("${PROJECT_NAME}-app" PRIVATE
            -pedantic
            -Wall
            -Wextra
            -Wfatal-errors
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options("${PROJECT_NAME}-app" PRIVATE
            /Wall
    )
endif()

target_include_directories("${PROJECT_NAME}-app" PRIVATE $<BUILD_INTERFACE:${SHARED_HEADERS_DIR}>)
target_include_directories("${PROJECT_NAME}-app" PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/shared/>)
target_include_directories("${PROJECT_NAME}-app" PUBLIC $<BUILD_INTERFACE:${TOOL_COMMON_INCLUDE_DIRS}>)

find_package(dxcompiler REQUIRED)
target_link_libraries("${PROJECT_NAME}-app" dxcompiler::dxcompiler)

find_package(Boost COMPONENTS)
target_link_libraries("${PROJECT_NAME}-app" boost::boost)

target_link_libraries("${PROJECT_NAME}-app" "${PROJECT_NAME}-lib")

find_package(metrix REQUIRED)
target_link_libraries("${PROJECT_NAME}-app" metrix::metrix)

install(TARGETS
        ${PROJECT_NAME}-app
        EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
