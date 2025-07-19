cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

function(honesty_discover_tests TARGET)
	cmake_parse_arguments(
		""
		""
		"WORKING_DIRECTORY;DISCOVERY_TIMEOUT"
		""
		${ARGN}
	)

	# Handle argument defaults
	if(NOT _WORKING_DIRECTORY)
		set(_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	endif()

	if(NOT _DISCOVERY_TIMEOUT)
		set(_DISCOVERY_TIMEOUT 5)
	endif()

	# Check to see if a counter property exists
	get_property(
		counterFound
		TARGET ${TARGET}
		PROPERTY CTEST_DISCOVERED_TEST_COUNTER
		SET
	)

	# Either increment a counter extracted from the target or initialize a new counter
	if(counterFound)
		get_property(
			counter
			TARGET ${TARGET}
			PROPERTY CTEST_DISCOVERED_TEST_COUNTER
		)
		math(EXPR counter "${counter} + 1")

	else()
		set(counter 1)
	endif()

	# Set the counter on the target
	set_property(
		TARGET ${TARGET}
		PROPERTY CTEST_DISCOVERED_TEST_COUNTER
		${counter}
	)

	# The files used to store test listings
	set(ctest_file_base "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_${counter}")
	set(ctest_include_file "${ctest_file_base}_include.cmake")

	get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL
		PROPERTY GENERATOR_IS_MULTI_CONFIG
	)

	# Set our output file name
	if(GENERATOR_IS_MULTI_CONFIG)
		set(ctest_tests_file "${ctest_file_base}_tests-$<CONFIG>.cmake")
	else()
		set(ctest_tests_file "${ctest_file_base}_tests.cmake")
	endif()

	set(ctest_tests_helper "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../script/honestyAddTests.cmake")

	# CMake functionality that will be injected prior to target execution.
	# As a result, test discovery will happen in the same configured context as the executable
	string(CONCAT ctest_include_content
		"if(EXISTS \"$<TARGET_FILE:${TARGET}>\")" "\n"
		"   if(NOT EXISTS \"${ctest_tests_file}\" OR" "\n"
		"     NOT \"${ctest_tests_file}\" IS_NEWER_THAN \"$<TARGET_FILE:${TARGET}>\" OR  \n"
		"     NOT \"${ctest_tests_file}\" IS_NEWER_THAN \"\${CMAKE_CURRENT_LIST_FILE}\") \n"
		"       include(\"${ctest_tests_helper}\")" "\n"
		"       honesty_extract_tests(" "\n"
		"           EXECUTABLE $<TARGET_FILE:${TARGET}>" "\n"
		"           CTEST_FILE ${ctest_tests_file}" "\n"
		"           WORKING_DIR ${_WORKING_DIRECTORY}" "\n"
		"           DISCOVERY_TIMEOUT ${_DISCOVERY_TIMEOUT}" "\n"
		"       )" "\n"
		"   endif()" "\n"
		"   include(\"${ctest_tests_file}\")" "\n"
		"else()" "\n"
		"   add_test(${TARGET}_NOT_BUILT ${TARGET}_NOT_BUILT)" "\n"
		"endif()" "\n"
	)

	if(GENERATOR_IS_MULTI_CONFIG)
		foreach(_config ${CMAKE_CONFIGURATION_TYPES})
			file(GENERATE OUTPUT "${ctest_file_base}_include-${_config}.cmake" CONTENT "${ctest_include_content}" CONDITION $<CONFIG:${_config}>)
		endforeach()

		file(WRITE "${ctest_include_file}" "include(\"${ctest_file_base}_include-\${CTEST_CONFIGURATION_TYPE}.cmake\")")
	else()
		file(GENERATE OUTPUT "${ctest_file_base}_include.cmake" CONTENT "${ctest_include_content}")
		file(WRITE "${ctest_include_file}" "include(\"${ctest_file_base}_include.cmake\")")
	endif()

	# Add discovered tests to directory TEST_INCLUDE_FILES
	set_property(DIRECTORY
		APPEND PROPERTY TEST_INCLUDE_FILES ${ctest_include_file}
	)
endfunction()
