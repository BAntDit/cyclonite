
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.errors import ConanInvalidConfiguration

class CycloniteRecipe(ConanFile):
    name = "cyclonite"
    version = "0.5.0.0"
    url = "https://github.com/BAntDit/cyclonite"
    description = "Cyclonite is a graphics engine."

    options = {
        "platform": ["x11", "wayland", "xcb", "mir", "windows", "android", "auto"],  # Let users choose
    }

    default_options = {
        "platform": "auto"
    }

    settings = "os", "compiler", "arch", "build_type"

    export_sources = "CMakeLists.txt", "*.cmake", ".clang-format", ".md", "src/*.h", "tests/*.h", "cmake/*"

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.10]")
        self.tool_requires("glslang/[~11.7]")
        if self.settings.compiler != "msvc":
            self.tool_requires("ninja/[>=1.11.0]")

    def requirements(self):
        self.requires("gtest/[~1.16]")
        self.requires("metrix/[~1.5]")
        self.requires("taskweaver/[~0.3]")
        self.requires("enttx/4.0.4.0")
        self.requires("glm/1.0.1")
        self.requires("sdl/3.2.6")
        self.requires("vulkan-validationlayers/1.3.243.0")
        self.requires("vulkan-loader/1.3.243.0")
        self.requires("vulkan-headers/1.3.243.0")
        self.requires("glslang/1.3.243.0")
        self.requires("spirv-tools/1.3.243.0")
        self.requires("spirv-headers/1.3.243.0")
        self.requires("boost/1.87.0")
        self.requires("nlohmann_json/3.12.0")

    def configure(self):
        self.settings.compiler.cppstd = "20"

    def package_id(self):
        self.info.settings.compiler.cppstd in ["20", "gnu20"]

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)

        if self.settings.compiler == "msvc":
            tc.generator = "Visual Studio 17 2022"
        else:
            tc.generator = "Ninja"

        if self.options.platform != "auto":
            if self.options.platform == "x11":
                tc.variables["VK_USE_PLATFORM_XLIB_KHR"] = True
            elif self.options.platform == "wayland":
                tc.variables["VK_USE_PLATFORM_WAYLAND_KHR"] = True
            elif self.options.platform == "xcb":
                tc.variables["VK_USE_PLATFORM_XCB_KHR"] = True
            elif self.options.platform == "mir":
                tc.variables["VK_USE_PLATFORM_MIR_KHR"] = True
            elif self.options.platform == "android":
                tc.variables["VK_USE_PLATFORM_ANDROID_KHR"] = True
            elif self.options.platform == "windows":
                tc.variables["VK_USE_PLATFORM_WIN32_KHR"] = True
            else:
                raise ConanInvalidConfiguration("Unexpected platform name.")


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

