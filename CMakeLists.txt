cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

project(synodic-template-library
	VERSION 0.1.0
	DESCRIPTION "Synodic Template Library"
	LANGUAGES CXX
)

# Enable CTest for the project
include(CTest)

# Find Conan dependencies
find_package(honesty REQUIRED)

# Provide a namespaced alias
add_library(synodic-template-library STATIC)
add_library(synodic::synodic-template-library ALIAS synodic-template-library)

set_target_properties(synodic-template-library
	PROPERTIES
	LINKER_LANGUAGE CXX
	CXX_STANDARD 23
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS NO
)

# Link dependencies
target_link_libraries(synodic-template-library)

add_subdirectory(
	library
)

if(BUILD_TESTING)
	add_subdirectory(
		tests
	)
endif()
