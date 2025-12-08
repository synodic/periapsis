target_sources(synodic-template-library
	PUBLIC
	FILE_SET CXX_MODULES
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
	"structure"
)

add_subdirectory(
	"math"
)

add_subdirectory(
	"statistics"
)

add_subdirectory(
	"utility"
)
