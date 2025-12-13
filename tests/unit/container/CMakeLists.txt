target_sources(tests
	PRIVATE
	test_queue.cpp
)

add_subdirectory(
	sparse
)

add_subdirectory(
	lockfree
)
add_subdirectory(
	dynamic
)