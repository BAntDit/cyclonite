
from conan import ConanFile
from conan.tools.files import get, copy
import os

class DxcConan(ConanFile):
    name = "dxcompiler"
    version = "1.8.2505"

    settings = "os", "compiler", "build_type", "arch"

    def source(self):
        self.run("git clone --recursive https://github.com/microsoft/DirectXShaderCompiler.git .")

    def build(self):
        build_dir = os.path.join(self.build_folder, "build")
        os.makedirs(build_dir, exist_ok=True)

        self.run(f"cmake -C ../cmake/caches/PredefinedParams.cmake -DCMAKE_BUILD_TYPE={self.settings.build_type} ..",
                 cwd=build_dir)

        self.run(f"cmake --build {build_dir} --target dxcompiler -j {os.cpu_count()}",
                 cwd=self.source_folder)

    def package(self):
        build_dir = os.path.join(self.build_folder, "build")

        # Copy built libraries
        if self.settings.os == "Windows":
            copy(self, "*.lib",
                 os.path.join(build_dir, "lib", str(self.settings.build_type)),
                 os.path.join(self.package_folder, "lib"))
            copy(self, "*.dll",
                 os.path.join(build_dir, "bin", str(self.settings.build_type)),
                 os.path.join(self.package_folder, "bin"))
        else:
            copy(self, "*.a",
                 os.path.join(build_dir, "lib"),
                 os.path.join(self.package_folder, "lib"))
            copy(self, "*.so*",
                 os.path.join(build_dir, "lib"),
                 os.path.join(self.package_folder, "lib"))

        # Copy headers
        copy(self, "*.h",
             os.path.join(self.source_folder, "include"),
             os.path.join(self.package_folder, "include"))

    def package_info(self):
        self.cpp_info.libs = ["dxcompiler"]