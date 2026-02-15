
file(GLOB TOOLS_SHADER_COMPILER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library("${PROJECT_NAME}-lib"
        ${ALL_SHARED_HEADERS}
        ${TOOLS_SHADER_COMPILER_HEADERS}
        ${TOOLS_SHADER_COMPILER_SOURCES})

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

find_package(dxcompiler REQUIRED)
target_link_libraries("${PROJECT_NAME}-lib" PRIVATE dxcompiler::dxcompiler)

find_package(DirectX-Headers REQUIRED)
target_link_libraries("${PROJECT_NAME}-lib" PRIVATE Microsoft::DirectX-Headers)

find_package(metrix REQUIRED)
target_link_libraries("${PROJECT_NAME}-lib" PRIVATE metrix::metrix)

# Install the library
install(TARGETS ${PROJECT_NAME}-lib
        EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/includes/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}-lib
        FILES_MATCHING PATTERN "*.h"
)

# Install the export set
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

