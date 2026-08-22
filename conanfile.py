
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.errors import ConanInvalidConfiguration
from conan.tools.files import copy
from conan.tools.env import VirtualRunEnv, VirtualBuildEnv
import os

class CycloniteRecipe(ConanFile):
    name = "cyclonite"
    version = "0.6.0.0"
    url = "https://github.com/BAntDit/cyclonite"
    description = "Cyclonite is a graphics engine."

    options = {
        "platform": [
            "linux-x11",
            "linux-wayland",
            "linux-xcb",
            "linux-mir",
            "windows",
            "android",
            "auto"
        ],  # Let users choose
        "gapi": [
            "vulkan",
            "d3d12"
        ]
    }

    default_options = {
        "platform": "auto",
        "gapi": "vulkan"
    }

    settings = "os", "compiler", "arch", "build_type"

    def export_sources(self):
        copy(self, "CMakeLists.txt", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "*.cmake", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, ".clang-format", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "src/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "tests/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "examples/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "cmake/*", src=self.recipe_folder, dst=self.export_sources_folder)
        #copy(self, "tools/*", src=self.recipe_folder, dst=self.export_sources_folder, excludes=["*/.vs/*", "*/cmake-build-*/*", "*/build/*", "*/__pycache__/*"])
        copy(self, "shared/*", src=self.recipe_folder, dst=self.export_sources_folder)

    def build_requirements(self):
        self.tool_requires("glslang/[~11.7]")
        if self.settings.compiler != "msvc":
            self.tool_requires("ninja/[>=1.11.0]")

        self.tool_requires("shader-compiler/0.1.3.0")

    def requirements(self):
        self.requires("gtest/[~1.16]")
        self.requires("metrix/1.8.2.0")
        self.requires("glm/1.0.1")
        self.requires("sdl/3.2.6")

        if self.options.gapi == "vulkan":
            self.requires("spirv-headers/1.3.243.0")
            self.requires("spirv-tools/1.3.243.0")
            self.requires("glslang/1.3.243.0")
            self.requires("vulkan-validationlayers/1.3.243.0")
            self.requires("vulkan-loader/1.3.243.0")
            self.requires("vulkan-headers/1.3.243.0")
            self.requires("vulkan-memory-allocator/3.0.1")

        self.requires("boost/1.87.0")
        self.requires("nlohmann_json/3.12.0")

    def configure(self):
        self.settings.compiler.cppstd = "20"

    def package_id(self):
        self.info.settings.compiler.cppstd in ["20", "gnu20"]

    def generate(self):
        deps = CMakeDeps(self)

        if self.settings.os == "Windows":
            deps.set_property("vulkan-memory-allocator", "cmake_file_name", "vulkan-memory-allocator")
            deps.set_property("vulkan-memory-allocator", "cmake_target_name", "vulkan-memory-allocator::vulkan-memory-allocator")

        deps.generate()

        if self.settings.os == "Windows":
            run_env = VirtualRunEnv(self)
            run_env.generate()
            build_env = VirtualBuildEnv(self)
            build_env.generate()

        tc = CMakeToolchain(self)
        tc.generator = "Ninja"

        if self.options.platform != "auto":
            if self.options.platform == "linux-x11":
                tc.variables["PLATFORM_LINUX_X11"] = True
            elif self.options.platform == "linux-wayland":
                tc.variables["PLATFORM_LINUX_WAYLAND"] = True
            elif self.options.platform == "linux-xcb":
                tc.variables["PLATFORM_LINUX_XCB"] = True
            elif self.options.platform == "linux-mir":
                tc.variables["PLATFORM_LINUX_MIR"] = True
            elif self.options.platform == "android":
                tc.variables["PLATFORM_ANDROID"] = True
            elif self.options.platform == "windows":
                tc.variables["PLATFORM_WINDOWS"] = True
            else:
                raise ConanInvalidConfiguration("Unexpected platform name.")

        if self.options.gapi == "vulkan":
            tc.variables["GAPI_VULKAN"] = True
        elif self.options.gapi == "d3d12":
            tc.variables["GAPI_D3D12"] = True
        else:
            raise ConanInvalidConfiguration("Unsupported graphics API.")


        local_shared_dir = os.path.join(self.source_folder, "shared")
        local_shared_dir = local_shared_dir.replace("\\", "/")

        tc.variables["LOCAL_SHARED_HEADERS_DIR"] = local_shared_dir

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

