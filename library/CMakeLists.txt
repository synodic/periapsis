target_sources(synodic-template-library
	PUBLIC
		FILE_SET CXX_MODULES
		BASE_DIRS
			"${PROJECT_SOURCE_DIR}/library"
		FILES
			"library.ixx"
)

add_subdirectory(
	"algorithm"
)

add_subdirectory(
	"container"
)

add_subdirectory(
	"exception"
)

add_subdirectory(
	"function"
)

add_subdirectory(
	"string"
)

add_subdirectory(
	"thread"
)

add_subdirectory(
	"type"
)

add_subdirectory(
	"utility"
)
