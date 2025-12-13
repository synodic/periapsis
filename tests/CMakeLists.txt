cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

project(tests
	DESCRIPTION "Test project for Periapsis"
	LANGUAGES CXX
)

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
	synodic::periapsis
	GTest::gtest
	GTest::gtest_main
)

add_subdirectory(
	unit
)
