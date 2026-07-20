
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.errors import ConanInvalidConfiguration
from conan.tools.files import copy, load, save
import os
import glob

class ToolsShaderCompilerRecipe(ConanFile):
    name = "shader-compiler"
    version = "0.1.2.0"
    description = "shader-compiler tool for cyclonite engine."

    settings = "os", "compiler", "arch", "build_type"
    export_sources = "CMakeLists.txt"

    def export_sources(self):
        copy(self, "CMakeLists.txt", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "*.cmake", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, ".clang-format", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "app/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "lib/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "includes/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "shared/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "external/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "cmake/*", src=self.recipe_folder, dst=self.export_sources_folder)
        shared_dir = os.path.join(self.recipe_folder, "..", "..", "shared")
        copy(self, "*.h", shared_dir,
             os.path.join(self.export_sources_folder, "shared"))

    def requirements(self):
        self.requires("dxcompiler/1.8.2505@")
        self.requires("directx-headers/1.618.2")
        self.requires("boost/1.87.0")
        self.requires("metrix/1.8.2.0")

    def configure(self):
        self.settings.compiler.cppstd = "20"

    def _fix_directx_headers_cmake(self):
        self.output.info(f">>> generators_folder: {self.generators_folder}")
        self.output.info(f">>> build_folder: {self.build_folder}")

        # Search recursively to find where the file actually is
        for root, dirs, files in os.walk(self.build_folder):
            for f in files:
                if "DirectX" in f and f.endswith(".cmake"):
                    full_path = os.path.join(root, f)
                    self.output.info(f">>> Found cmake file: {full_path}")
                    content = load(self, full_path)
                    if "d3dx12-format-properties" in content:
                        self.output.info(f">>> Patching: {full_path}")
                        patched = content.replace("d3dx12-format-properties", "")
                        save(self, full_path, patched)
                        self.output.info(f">>> Patched successfully: {full_path}")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        if self.settings.os == "Windows":
            self._fix_directx_headers_cmake()

        tc = CMakeToolchain(self)
        if self.settings.compiler == "msvc":
            tc.generator = "Visual Studio 17 2022"
        else:
            tc.generator = "Ninja"

        shared_dir = os.path.join(self.recipe_folder, "..", "..", "shared")
        shared_dir = shared_dir.replace("\\", "/")

        tc.variables["SHARED_HEADERS_DIR"] = shared_dir
        tc.variables["REQUIRED_CXX_STANDARD"] = "20"
        tc.generate()

    def build(self):
        if self.settings.os == "Windows":
            self._fix_directx_headers_cmake()

        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        dx = self.dependencies["dxcompiler"]
        self.output.info(f"dxcompiler package_folder: {dx.package_folder}")
        self.output.info(f"dxcompiler cpp_info.libdirs: {dx.cpp_info.libdirs}")

        # Find and copy the dxcompiler config files from build directory to package
        # Look for dxcompiler config files in common locations
        possible_config_dirs = [
            os.path.join(self.build_folder, "dxcompiler-config"),
            os.path.join(self.build_folder, "cmake", "dxcompiler"),
            os.path.join(self.build_folder, "lib", "cmake", "dxcompiler"),
            self.build_folder  # root directory
        ]

        config_copied = False
        for config_dir in possible_config_dirs:
            if os.path.exists(config_dir):
                config_files = [
                    "dxcompilerConfig.cmake",
                    "dxcompiler-config.cmake"
                ]
                for config_file in config_files:
                    src_path = os.path.join(config_dir, config_file)
                    if os.path.exists(src_path):
                        # Copy to a standard location in the package
                        dst_dir = os.path.join(self.package_folder, "lib", "cmake", "dxcompiler")
                        os.makedirs(dst_dir, exist_ok=True)
                        copy(self,
                             pattern=config_file,
                             src=config_dir,
                             dst=os.path.join("lib", "cmake", "dxcompiler"))
                        config_copied = True
                        self.output.info(f"Copied {config_file} to package")
        if not config_copied:
            self.output.warning("Could not find dxcompiler config files to copy")

        dxcompiler_cpp_info = self.dependencies["dxcompiler"].cpp_info
        for libdir in dxcompiler_cpp_info.libdirs:
            copy(self, "libdxcompiler.so*", src=libdir,
                 dst=os.path.join(self.package_folder, "lib"))

    def package_info(self):
        # Library component
        (self.cpp_info.components["shader-compiler-lib"]
            .set_property("cmake_target_name", "shader-compiler::shader-compiler-lib"))
        self.cpp_info.components["shader-compiler-lib"].libs = ["shader-compiler-lib"]
        self.cpp_info.components["shader-compiler-lib"].includedirs = ["include"]
        self.cpp_info.components["shader-compiler-lib"].bindirs = []

        # Add requirements for the library component
        self.cpp_info.components["shader-compiler-lib"].requires = [
            "dxcompiler::dxcompiler",
            "directx-headers::directx-headers",
            "metrix::metrix"
        ]

        # Executable component
        exe_name = "shader-compiler-app"
        if self.settings.build_type == "Debug":
            exe_name += "_d"
        if self.settings.os == "Windows":
            exe_name += ".exe"

        (self.cpp_info.components["shader-compiler-exe"]
            .set_property("cmake_target_name", "shader-compiler::shader-compiler-app"))
        self.cpp_info.components["shader-compiler-exe"].libs = []
        self.cpp_info.components["shader-compiler-exe"].includedirs = []
        self.cpp_info.components["shader-compiler-exe"].bindirs = ["bin"]

        # Define the executable path as a property
        self.cpp_info.components["shader-compiler-exe"].set_property(
            "conan:executable",
            exe_name
        )

        # Add boost requirement for executable
        self.cpp_info.components["shader-compiler-exe"].requires = [
            "shader-compiler-lib",
            "boost::boost",
            "dxcompiler::dxcompiler",
            "directx-headers::directx-headers",
            "metrix::metrix"
        ]

        self.cpp_info.set_property("cmake_target_name", "shader-compiler::shader-compiler")

        # Add the bin directory to PATH for consumers
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))

    def deploy(self):
        copy(self, "libdxcompiler.so*",
             src=self.dependencies["dxcompiler"].cpp_info.libdirs[0],
             dst=".")
