from conan import ConanFile
from conan.tools.files import copy, save, download, unzip
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
import os
import platform

class DxCompilerConan(ConanFile):
    name = "dxcompiler"
    version = "1.8.2505"
    license = "MIT"
    url = "https://github.com/microsoft/DirectXShaderCompiler"
    description = "DirectX Shader Compiler (DXC)"
    settings = "os", "compiler", "build_type", "arch"

    # Windows-specific properties for pre-built binaries
    _prebuilt_url = "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2505.1/dxc_2025_07_14.zip"

    @property
    def _is_windows(self):
        return platform.system() == "Windows"

    def source(self):
        if self._is_windows:
            # For Windows, we'll download pre-built binaries in the package() method
            pass
        else:
            self.run("git clone --recursive https://github.com/microsoft/DirectXShaderCompiler.git .")

    def generate(self):
        if self._is_windows:
            tc = CMakeToolchain(self)
            # Add predefined params cache
            predefined_cache = os.path.join(self.source_folder, "cmake", "caches", "PredefinedParams.cmake")
            tc.cache_variables["CMAKE_TOOLCHAIN_FILE"] = predefined_cache  # passes it as initial cache
            tc.generate()

            # Generate deps (empty if no dependencies)
            deps = CMakeDeps(self)
            deps.generate()

    def build(self):
        if self._is_windows:
            # No build needed for Windows - we use pre-built binaries
            self.output.info("Using pre-built binaries for Windows")
            return

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

        if self._is_windows:
            content = """
include(CMakeFindDependencyMacro)

if(NOT TARGET dxcompiler::dxcompiler)
    add_library(dxcompiler::dxcompiler SHARED IMPORTED)
    set_target_properties(dxcompiler::dxcompiler PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/include"
    )
    
    # Windows specific configuration
    if(WIN32)
        set_target_properties(dxcompiler::dxcompiler PROPERTIES
            IMPORTED_LOCATION "${PACKAGE_PREFIX_DIR}/bin/dxcompiler.dll"
            IMPORTED_IMPLIB "${PACKAGE_PREFIX_DIR}/lib/dxcompiler.lib"
        )
    endif()
endif()
"""
        else:
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

    def _download_windows_prebuilt(self):
        """Download pre-built DXC binaries for Windows"""
        download_url = self._prebuilt_url;
        
        self.output.info(f"Downloading pre-built DXC from: {download_url}")
        
        zip_path = os.path.join(self.build_folder, "dxc.zip")
        download(self, download_url, zip_path)
        
        extract_folder = os.path.join(self.build_folder, "extracted")
        unzip(self, zip_path, extract_folder)
        
        return extract_folder

    def package(self):
        if self._is_windows:
            extract_folder = self._download_windows_prebuilt()
            
            copy(self, "*.h", 
                 src=os.path.join(extract_folder, "inc"), 
                 dst=os.path.join(self.package_folder, "include"))
            
            lib_src_folder = extract_folder
            if self.settings.arch == "x86_64":
                lib_src_folder = os.path.join(extract_folder, "lib", "x64")
            elif self.settings.arch == "x86":
                lib_src_folder = os.path.join(extract_folder, "lib", "x86")

            bin_src_folder = extract_folder
            if self.settings.arch == "x86_64":
                bin_src_folder = os.path.join(extract_folder, "bin", "x64")
            elif self.settings.arch == "x86":
                bin_src_folder = os.path.join(extract_folder, "bin", "x86")
            
            # Copy .lib files
            copy(self, "*.lib", 
                 src=lib_src_folder, 
                 dst=os.path.join(self.package_folder, "lib"))
            
            # Copy .dll files
            copy(self, "*.dll", 
                 src=bin_src_folder, 
                 dst=os.path.join(self.package_folder, "bin"))
        else:
            build_dir = os.path.join(self.build_folder, "build")

            copy(self, "*.h", src=os.path.join(self.source_folder, "include"), dst=os.path.join(self.package_folder, "include"))

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

        if self._is_windows:
            self.cpp_info.bindirs = ["bin"]

        # So CMake can find dxcompilerConfig.cmake inside package
        self.cpp_info.builddirs = ["lib/cmake"]

        # Conan 2 properties for CMake
        self.cpp_info.set_property("cmake_file_name", "dxcompiler")
        self.cpp_info.set_property("cmake_target_name", "dxcompiler::dxcompiler")
