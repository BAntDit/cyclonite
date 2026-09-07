
file(GLOB TOOLS_ASSET_TOOL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable("${PROJECT_NAME}-console" ${TOOLS_ASSET_TOOL_HEADERS} ${TOOLS_ASSET_TOOL_SOURCES})

set_target_properties("${PROJECT_NAME}-console" PROPERTIES
        VERSION ${PROJECT_VERSION}

        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS OFF

        DEBUG_POSTFIX _d

        INSTALL_RPATH "$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options("${PROJECT_NAME}-console" PRIVATE
            -pedantic
            -Wall
            -Wextra
            -Wfatal-errors
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options("${PROJECT_NAME}-console" PRIVATE
            /Wall
    )
endif()

target_include_directories("${PROJECT_NAME}-console" PRIVATE $<BUILD_INTERFACE:${SHARED_HEADERS_DIR}>)
target_include_directories("${PROJECT_NAME}-console" PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/shared/>)
target_include_directories("${PROJECT_NAME}-console" PUBLIC $<BUILD_INTERFACE:${TOOL_COMMON_INCLUDE_DIRS}>)

find_package(metrix REQUIRED)
target_link_libraries("${PROJECT_NAME}-console" PRIVATE metrix::metrix)

find_package(Boost COMPONENTS)
target_link_libraries("${PROJECT_NAME}-console" PRIVATE boost::boost)
