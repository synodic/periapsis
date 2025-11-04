target_sources(synodic-template-library
	PUBLIC
		FILE_SET CXX_MODULES
		BASE_DIRS
			"${PROJECT_SOURCE_DIR}/library"
		FILES
			"shared.ixx"
			"container_c.ixx"
			"sequence_container.ixx"
			"associative_container.ixx"
))
