from conan import ConanFile
from conan.tools.files import copy, save
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
import os

class DxCompilerConan(ConanFile):
    name = "dxcompiler"
    version = "1.8.2505"
    license = "MIT"
    url = "https://github.com/microsoft/DirectXShaderCompiler"
    description = "DirectX Shader Compiler (DXC)"
    settings = "os", "compiler", "build_type", "arch"

    def source(self):
        self.run("git clone --recursive https://github.com/microsoft/DirectXShaderCompiler.git .")

    def generate(self):
        tc = CMakeToolchain(self)
        # Add predefined params cache
        predefined_cache = os.path.join(self.source_folder, "cmake", "caches", "PredefinedParams.cmake")
        tc.cache_variables["CMAKE_TOOLCHAIN_FILE"] = predefined_cache  # passes it as initial cache
        tc.generate()

        # Generate deps (empty if no dependencies)
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        build_dir = os.path.join(self.build_folder, "build")
        os.makedirs(build_dir, exist_ok=True)

        predefined_cache = os.path.join(self.source_folder, "cmake", "caches", "PredefinedParams.cmake")

        # Configure using the DXC required predefined cache
        self.run(f'cmake -C "{predefined_cache}" -B "{build_dir}" -S "{self.source_folder}"')

        # Build the dxcompiler target
        self.run(f'cmake --build "{build_dir}" --target dxcompiler -j {os.cpu_count()}')

    def _create_cmake_config(self):
        config_dir = os.path.join(self.package_folder, "lib", "cmake", "dxcompiler")
        os.makedirs(config_dir, exist_ok=True)

        content = """
include(CMakeFindDependencyMacro)

if(NOT TARGET dxcompiler::dxcompiler)
    add_library(dxcompiler::dxcompiler SHARED IMPORTED)
    set_target_properties(dxcompiler::dxcompiler PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/include"
        IMPORTED_LOCATION "${PACKAGE_PREFIX_DIR}/lib/libdxcompiler.so"
    )
    if(WIN32)
        set_target_properties(dxcompiler::dxcompiler PROPERTIES
            IMPORTED_IMPLIB "${PACKAGE_PREFIX_DIR}/lib/dxcompiler.lib"
        )
    endif()
endif()
"""
        save(self, os.path.join(config_dir, "dxcompilerConfig.cmake"), content)

    def package(self):
        build_dir = os.path.join(self.build_folder, "build")

        # Copy headers
        copy(self, "*.h", src=os.path.join(self.source_folder, "include"), dst=os.path.join(self.package_folder, "include"))

        # Copy libraries
        if self.settings.os == "Windows":
            copy(self, "*.lib", src=os.path.join(build_dir, "lib", str(self.settings.build_type)), dst=os.path.join(self.package_folder, "lib"))
            copy(self, "*.dll", src=os.path.join(build_dir, "bin", str(self.settings.build_type)), dst=os.path.join(self.package_folder, "bin"))
        else:
            copy(self, "*.a", src=os.path.join(build_dir, "lib"), dst=os.path.join(self.package_folder, "lib"))
            copy(self, "*.so*", src=os.path.join(build_dir, "lib"), dst=os.path.join(self.package_folder, "lib"))

        # Generate CMake config for consumers
        self._create_cmake_config()

    def package_info(self):
        self.cpp_info.libs = ["dxcompiler"]
        self.cpp_info.includedirs = ["include"]

        # So CMake can find dxcompilerConfig.cmake inside package
        self.cpp_info.builddirs = ["lib/cmake"]

        # Conan 2 properties for CMake
        self.cpp_info.set_property("cmake_file_name", "dxcompiler")
        self.cpp_info.set_property("cmake_target_name", "dxcompiler::dxcompiler")
