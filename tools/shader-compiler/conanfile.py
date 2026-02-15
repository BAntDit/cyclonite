
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.errors import ConanInvalidConfiguration
from conan.tools.files import copy
import os

class ToolsShaderCompilerRecipe(ConanFile):
    name = "shader-compiler"
    version = "0.0.1.0"
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

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        if self.settings.compiler == "msvc":
            tc.generator = "Visual Studio 17 2022"
        else:
            tc.generator = "Ninja"

        tc.variables["SHARED_HEADERS_DIR"] = os.path.join(self.recipe_folder, "..", "..", "shared")
        tc.variables["REQUIRED_CXX_STANDARD"] = "20"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

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