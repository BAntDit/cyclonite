
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps
from conan.tools.files import copy
import os

class AssetToolRecipe(ConanFile):
    name = "asset-tool"
    version = "0.1.0.0"
    description = "asset tool for cyclonite engine."

    settings = "os", "compiler", "arch", "build_type"

    def export_sources(self):
        copy(self, "CMakeLists.txt", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "app/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "lib/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "includes/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "shared/*", src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, "cmake/*", src=self.recipe_folder, dst=self.export_sources_folder)
        shared_dir = os.path.join(self.recipe_folder, "..", "..", "shared")
        copy(self, "*.h", shared_dir,
             os.path.join(self.export_sources_folder, "shared"))

    def requirements(self):
        self.requires("boost/1.87.0")
        self.requires("metrix/1.8.2.0")
        self.requires("tinygltf/2.9.7")

    def configure(self):
        self.settings.compiler.cppstd = "20"

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.generator = "Ninja"

        shared_dir = os.path.join(self.recipe_folder, "..", "..", "shared")
        shared_dir = shared_dir.replace("\\", "/")

        tc.variables["SHARED_HEADERS_DIR"] = shared_dir
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
        (self.cpp_info.components["asset-tool-lib"]
            .set_property("cmake_target_name", "asset-tool::asset-tool-lib"))
        self.cpp_info.components["asset-tool-lib"].libs = ["asset-tool-lib"]
        self.cpp_info.components["asset-tool-lib"].includedirs = ["include"]
        self.cpp_info.components["asset-tool-lib"].bindirs = []
        self.cpp_info.components["asset-tool-lib"].requires = ["metrix::metrix"]

        exe_name = "asset-tool-console"
        if self.settings.build_type == "Debug":
            exe_name += "_d"
        if self.settings.os == "Windows":
            exe_name += ".exe"

        (self.cpp_info.components["asset-tool-exe"]
            .set_property("cmake_target_name", "asset-tool::asset-tool-console"))
        self.cpp_info.components["asset-tool-exe"].libs = []
        self.cpp_info.components["asset-tool-exe"].includedirs = []
        self.cpp_info.components["asset-tool-exe"].bindirs = ["bin"]

        # Define the executable path as a property
        self.cpp_info.components["asset-tool-exe"].set_property("conan:executable", exe_name)

        self.cpp_info.components["asset-tool-exe"].requires = [
            "asset-tool-lib",
            "boost::boost",
            "metrix::metrix"
        ]

        self.cpp_info.set_property("cmake_target_name", "asset-tool::asset-tool")

        # Add the bin directory to PATH for consumers
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))
        