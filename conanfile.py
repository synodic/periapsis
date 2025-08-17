from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy

class MyProject(ConanFile):
	name = "myproject"
	version = "1.0"
	settings = "os", "compiler", "build_type", "arch"
	requires = ["honesty/[>=0.0.0.dev1]"]

	def layout(self):
		cmake_layout(self)

	def generate(self):
		deps = CMakeDeps(self)
		deps.generate()
		tc = CMakeToolchain(self)
		tc.user_presets_path = None
		tc.generate()

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		cmake = CMake(self)
		cmake.install()

	def package_info(self):
		self.cpp_info.libs = ["honesty"]

	def export_sources(self):
		copy(self, "CMakeLists.txt", src=self.recipe_folder, dst=self.export_sources_folder)
		copy(self, "src/*", src=self.recipe_folder, dst=self.export_sources_folder)
		copy(self, "cmake/*", src=self.recipe_folder, dst=self.export_sources_folder)
		copy(self, "tests/*", src=self.recipe_folder, dst=self.export_sources_folder)
