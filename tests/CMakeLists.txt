cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

project(tests
	DESCRIPTION "Test project for Synodic Template Library"
	LANGUAGES CXX
)

# Find Conan dependencies
find_package(honesty REQUIRED)

add_executable(tests)

set_target_properties(tests
	PROPERTIES
		LINKER_LANGUAGE CXX
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS NO
)

target_link_libraries(tests
	PRIVATE
	synodic::honesty::main
)

add_subdirectory(
	"unit"
)
