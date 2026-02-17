cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

# Enable std module support for MSVC - MUST be before project()
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_MODULE_STD ON)

project(periapsis
	VERSION 0.1.0
	DESCRIPTION "A supplementary C++ standard library collection"
	LANGUAGES CXX
)

option(BUILD_TESTING "Build the tests verifying GTest functionality" ON)

# Find Conan dependencies
find_package(GTest REQUIRED)

# Provide a namespaced alias
add_library(periapsis STATIC)
add_library(synodic::periapsis ALIAS periapsis)

set_target_properties(periapsis
	PROPERTIES
	LINKER_LANGUAGE CXX
	CXX_STANDARD 23
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS NO
)

target_compile_features(periapsis PUBLIC cxx_std_23)

# Link dependencies
target_link_libraries(periapsis)

add_subdirectory(
	src
)

# NOTE: For IDE setups that do not use CMake directly, we setup all IDE directories to mimic
# their system directory structure. See https://gitlab.kitware.com/cmake/cmake/-/issues/25992
file(GLOB_RECURSE file_list ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

# Must be called from the target definition directory. Only matched against 'target_sources'
# files (No nested CMakeList.txt, docs, etc).
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${file_list})

if(BUILD_TESTING AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
	# Do not include CTest because it adds targets and integrations we do not want to support
	enable_testing()

	include(GoogleTest)
	gtest_discover_tests(my_test_executable)

	add_subdirectory(
		tests
	)
endif()

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Generate package config files
write_basic_package_version_file(
	periapsisConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
	tool/cmake/periapsisConfig.cmake.in
	periapsisConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/periapsis
)

install(TARGETS periapsis
	EXPORT periapsisTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}

	FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/periapsis
)

# Create empty include directory to satisfy CMake's exported target requirements
# (module-only library with no headers, but CMake expects the directory to exist)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export and package configuration
install(EXPORT periapsisTargets
	FILE periapsisTargets.cmake
	NAMESPACE synodic::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/periapsis
)

install(
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/periapsisConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/periapsisConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/periapsis
)
