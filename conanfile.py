
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

class CycloniteRecipe(ConanFile):
    name = "cyclonite"
    version = "0.4.0.0"
    url = "https://github.com/BAntDit/cyclonite"
    description = "Cyclonite is a graphics engine."

    settings = "os", "compiler", "arch", "build_type"

    export_sources = "CMakeLists.txt", "*.cmake", ".clang-format", ".md", "src/*.h", "tests/*.h", "cmake/*"

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.10]")
        if self.settings.compiler != "msvc":
            self.tool_requires("ninja/[>=1.11.0]")

    def requirements(self):
        self.requires("gtest/[~1.16]")
        self.requires("metrix/[~1.5]")
        self.requires("taskweaver/[~0.3]")
        self.requires("enttx/4.0.4.0")
        self.requires("glm/1.0.1")
        self.requires("sdl/3.2.6")
        self.requires("vulkan-validationlayers/[~1.3]")
        self.requires("vulkan-loader/1.3.239.0")
        self.requires("vulkan-headers/[~1.3]", override=True)
        self.requires("glslang/[~11.7]")
        self.requires("spirv-tools/[~1.3]", override=True)
        self.requires("spirv-headers/[~1.3]", override=True)
        self.requires("boost/1.87.0")

    def configure(self):
        self.settings.compiler.cppstd = "20"

    def package_id(self):
        self.info.settings.compiler.cppstd in ["20", "gnu20"]

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)

        if self.settings.compiler == "msvc":
            tc.generator = "Visual Studio"
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
        self.cpp_info.set_property("cmake_file_name", "cyclonite")
        self.cpp_info.set_property("cmake_target_name", "cyclonite::cyclonite")

        self.cpp_info.libs = ["cyclonite"]

        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs = ["pthread"]

