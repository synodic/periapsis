target_sources(synodic-template-library
	PUBLIC
		FILE_SET CXX_MODULES
		BASE_DIRS
			"${PROJECT_SOURCE_DIR}/library"
		FILES
			"bit_map.ixx"
			"hash_map.ixx"
			"hash_set.ixx"
			"vector.ixx"
))
