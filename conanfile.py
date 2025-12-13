from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain
from conan.tools.files import copy

from conanfile_base import CPPythonBase


class MyProject(CPPythonBase):
    name = 'periapsis'
    version = '1.0'
    settings = 'os', 'compiler', 'build_type', 'arch'
    exports = 'conanfile_base.py'

    def layout(self):
        super().layout()

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.user_presets_path = False
        tc.generate()

    def requirements(self):
        super().requirements()

    def build_requirements(self):
        super().build_requirements()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property('cmake_find_mode', 'none')
        self.cpp_info.builddirs = ['.']

    def export_sources(self):
        copy(self, 'CMakeLists.txt', src=self.recipe_folder, dst=self.export_sources_folder)
        copy(self, 'src/*', src=self.recipe_folder, dst=self.export_sources_folder)
        copy(
            self,
            'tool/cmake/periapsisConfig.cmake.in',
            src=self.recipe_folder,
            dst=self.export_sources_folder,
        )
