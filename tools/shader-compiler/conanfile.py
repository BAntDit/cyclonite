
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.errors import ConanInvalidConfiguration

class ToolsShaderCompilerRecipe(ConanFile):
    name = "shader-compiler"
    version = "0.0.1.0"
    description = "shader-compiler tool for cyclonite engine."

    settings = "os", "compiler", "arch", "build_type"
    export_sources = "CMakeLists.txt", "includes/*", "lib/*", "external/*"

    def requirements(self):
        self.requires("dxcompiler/1.8.2505@")
        self.requires("boost/1.87.0")

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
        self.cpp_info.set_property("cmake_file_name", "shader-compiler")
        self.cpp_info.set_property("cmake_target_name", "shader-compiler")