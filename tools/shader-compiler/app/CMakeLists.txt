
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

target_include_directories("${PROJECT_NAME}-app" PUBLIC "${TOOL_COMMON_INCLUDE_DIRS}")

find_package(Boost COMPONENTS program_options REQUIRED)
#find_package(Boost COMPONENTS program_options REQUIRED)
#find_package(Boost COMPONENTS headers REQUIRED)
target_link_libraries("${PROJECT_NAME}-app" Boost::headers)
