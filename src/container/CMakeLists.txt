target_sources(synodic-template-library
	PUBLIC
		FILE_SET CXX_MODULES
		FILES
			"queue.ixx"
)

add_subdirectory(
	sparse
)

add_subdirectory(
	lockfree
)

