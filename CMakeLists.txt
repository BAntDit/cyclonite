cmake_minimum_required(VERSION 3.10)

project(cyclonite VERSION 0.4.0.0)

option(ENABLE_SIMD_AVX2 "enable SIMD AVX2" OFF)
option(ENABLE_SIMD_AVX "enable SIMD AVX" OFF)
option(DISABLE_SIMD "force disable SIMD" OFF)

set(PUBLIC_HEADERS
    src/config.h
    src/hash.h
    src/platform.h
    src/windowProperties.h
    src/options.h
    src/event.h
    src/eventReceivable.h
    src/input.h
    src/app.h
    src/root.h
    src/surface.h
    src/error.h
    src/typedefs.h
    src/compositor/nodeAsset.h
    src/animations/animation.h
    src/animations/sampler.h
    src/buffers/arena.h
    src/buffers/bufferView.h
    src/resources/contiguousData.h
    src/resources/staging.h
    src/resources/geometry.h
    src/resources/resourceState.h
    src/resources/resource.h
    src/resources/resourceManager.h
    src/resources/buffer.h
    src/compositor/nodeIdentifier.h
    src/compositor/nodeTypeRegister.h
    src/compositor/passType.h
    src/compositor/links.h
    src/compositor/workspace.h
    src/compositor/baseLogicNode.h
    src/compositor/baseGraphicsNode.h
    src/compositor/logicNode.h
    src/compositor/graphicsNode.h
    src/compositor/graphicsNodeBuilder.h
    src/compositor/logicNodeBuilder.h
    src/compositor/logicNodeInterface.h
    src/compositor/graphicsNodeInterface.h
    src/compositor/node.h
    src/compositor/frameCommands.h
    src/compositor/passIterator.h
    src/components/transform.h
    src/components/transformStorage.h
    src/components/mesh.h
    src/components/meshStorage.h
    src/components/camera.h
    src/components/animator.h
    src/components/animatorStorage.h
    src/systems/updateStages.h
    src/systems/transformSystem.h
    src/systems/meshSystem.h
    src/systems/cameraSystem.h
    src/systems/uniformSystem.h
    src/systems/renderSystem.h
    src/systems/animationSystem.h
    src/baseRenderTarget.h
    src/surfaceRenderTarget.h
    src/frameBufferRenderTarget.h
    src/sdl/sdlSupport.h
    src/sdl/sdlWindow.h
    src/vulkan/handle.h
    src/vulkan/sharedHandle.h
    src/vulkan/weakHandle.h
    src/vulkan/instance.h
    src/vulkan/device.h
    src/vulkan/baseCommandBufferSet.h
    src/vulkan/commandBufferSet.h
    src/vulkan/commandPool.h
    src/vulkan/baseSurface.h
    src/vulkan/xlibSurface.h
    src/vulkan/win32Surface.h
    src/vulkan/wlSurface.h
    src/vulkan/androidSurface.h
    src/vulkan/platformSurface.h
    src/vulkan/memoryPage.h
    src/vulkan/memoryManager.h
    src/vulkan/image.h
    src/vulkan/imageView.h
    src/vulkan/frameBuffer.h
    src/vulkan/buffer.h
    src/vulkan/shaderModule.h
    src/multithreading/lockFreeQueue.h
    src/multithreading/taskManager.h
    src/multithreading/task.h
    src/multithreading/taskPool.h
    src/multithreading/worker.h
    src/multithreading/render.h
    src/shaders/shaderDefinitions.h
    src/uuids.h
)

set(PRIVATE_HEADERS
    src/vulkan/internal/deviceCreationFunctions.h
    src/vulkan/internal/fillBufferCreationInfo.h
    src/vulkan/internal/fillImageCreationInfo.h
    src/vulkan/internal/imageHelpers.h
    src/animations/internal/interpolation.h
    src/multithreading/internal/utils.h
)

set(SOURCES
    src/windowProperties.cpp
    src/options.cpp
    src/eventReceivable.cpp
    src/input.cpp
    src/root.cpp
    src/surface.cpp
    src/baseRenderTarget.cpp
    src/frameBufferRenderTarget.cpp
    src/surfaceRenderTarget.cpp
    src/animations/animation.cpp
    src/animations/sampler.cpp
    src/resources/resource.cpp
    src/resources/buffer.cpp
    src/resources/geometry.cpp
    src/resources/staging.cpp
    src/resources/resourceManager.cpp
    src/compositor/node.cpp
    src/compositor/nodeIdentifier.cpp
    src/compositor/frameCommands.cpp
    src/compositor/passIterator.cpp
    src/compositor/workspaceBuilder.cpp
    src/compositor/links.cpp
    src/compositor/baseGraphicsNode.cpp
    src/compositor/graphicsNode.cpp
    src/compositor/baseLogicNode.cpp
    src/compositor/logicNodeInterface.cpp
    src/compositor/graphicsNodeInterface.cpp
    src/compositor/graphicsNodeBuilder.cpp
    src/compositor/workspace.cpp
    src/components/transform.cpp
    src/components/camera.cpp
    src/components/mesh.cpp
    src/components/animator.cpp
    src/components/animatorStorage.cpp
    src/systems/transformSystem.cpp
    src/systems/cameraSystem.cpp
    src/systems/meshSystem.cpp
    src/systems/uniformSystem.cpp
    src/systems/renderSystem.cpp
    src/systems/animationSystem.cpp
    src/sdl/sdlSupport.cpp
    src/sdl/sdlWindow.cpp
    src/vulkan/instance.cpp
    src/vulkan/device.cpp
    src/vulkan/commandPool.cpp
    src/vulkan/baseSurface.cpp
    src/vulkan/xlibSurface.cpp
    src/vulkan/win32Surface.cpp
    src/vulkan/wlSurface.cpp
    src/vulkan/androidSurface.cpp
    src/vulkan/memoryPage.cpp
    src/vulkan/memoryManager.cpp
    src/vulkan/image.cpp
    src/vulkan/imageView.cpp
    src/vulkan/buffer.cpp
    src/vulkan/frameBuffer.cpp
    src/vulkan/shaderModule.cpp
    src/multithreading/task.cpp
    src/multithreading/taskPool.cpp
    src/multithreading/render.cpp
    src/multithreading/worker.cpp
    src/multithreading/taskManager.cpp
    src/shaders/shaderDefinitions.cpp
)

set(SHADER_SOURCES
    src/shaders/default.vert
    src/shaders/default.frag
    src/shaders/screen.vert
    src/shaders/screen.frag
)

add_library(${PROJECT_NAME} SHARED
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
    ${SOURCES}
)

find_program(GLSLANG_VALIDATOR glslangValidator)

foreach(source ${SHADER_SOURCES})
    get_filename_component(dir "${source}" DIRECTORY)
    get_filename_component(name "${source}" NAME_WE)
    get_filename_component(ext "${source}" LAST_EXT)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${dir}")
    set(source "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${dir}/${name}${ext}.spv.cpp.txt")

    add_custom_command(
        OUTPUT "${output}"
        COMMAND "${GLSLANG_VALIDATOR}" -V ${source} -x -o "${output}"
        DEPENDS "${source}"
    )

    target_sources(${PROJECT_NAME} PRIVATE "${output}")
endforeach()
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src")

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
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

    if (ENABLE_SIMD_AVX2)
        target_compile_options(${PROJECT_NAME} PRIVATE -mavx2)
        target_compile_definitions(${PROJECT_NAME} PUBLIC ENABLED_SIMD_AVX2)
        message("-- enable SIMD AVX2")
    elseif(ENABLE_SIMD_AVX)
        target_compile_options(${PROJECT_NAME} PRIVATE -mavx)
        target_compile_definitions(${PROJECT_NAME} PUBLIC ENABLED_SIMD_AVX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /Wall
    )

    if (ENABLE_SIMD_AVX2)
        target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
        target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLED_SIMD_AVX2)
    elseif(ENABLE_SIMD_AVX)
        target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX)
        target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLED_SIMD_AVX)
    endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_NAME="${PROJECT_NAME}"
    VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VERSION_PATCH=${PROJECT_VERSION_PATCH}
    VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

find_package(glm REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_RADIANS)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)

target_link_libraries(${PROJECT_NAME} glm)
if (ENABLE_SIMD_AVX2)
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_AVX2)
elseif(ENABLE_SIMD_AVX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_AVX)
elseif(DISABLE_SIMD)
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_PURE)
endif()

target_link_libraries(${PROJECT_NAME} SDL2)

find_package(enttx REQUIRED)

find_package(easy-mp REQUIRED)

find_package(Boost REQUIRED COMPONENTS
    system
    program_options
)

target_link_libraries(${PROJECT_NAME}
    Boost::boost
    Boost::system
    Boost::program_options
)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

set(ALL_SOURCES_FOR_STYLE_CORRECTION ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${SOURCES})
file(GLOB_RECURSE ALL_EXAMPLES_SOURCES_FOR_STYLE_CORRECTION RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "examples/*.cpp")
file(GLOB_RECURSE ALL_EXAMPLES_HEADERS_FOR_STYLE_CORRECTION RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "examples/*.h")

list(
    APPEND ALL_SOURCES_FOR_STYLE_CORRECTION
        ${ALL_EXAMPLES_SOURCES_FOR_STYLE_CORRECTION}
        ${ALL_EXAMPLES_HEADERS_FOR_STYLE_CORRECTION}
)

include(ClangFormat.cmake)
CODE_STYLE_CORRECTION("${ALL_SOURCES_FOR_STYLE_CORRECTION}")

include(SubDirectoryList.cmake)

SUBDIRLIST(EXANPLEDIRS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
foreach(example ${EXANPLEDIRS})
    add_subdirectory("examples/${example}")
endforeach()

option(TESTS "whether needs to build tests" ON)

if (TESTS)
    find_package(GTest REQUIRED)

    enable_testing()

    add_subdirectory("tests")
endif()
