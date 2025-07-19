from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class MyProject(ConanFile):
    name = "myproject"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = ["honesty/[>=0.0.0.dev1]"]
    generators = "CMakeDeps"

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
