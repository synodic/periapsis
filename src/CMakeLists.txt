target_sources(periapsis
	PUBLIC
	FILE_SET CXX_MODULES
	FILES
	"library.ixx"
	"exception.ixx"
)

add_subdirectory(
	"algorithm"
)

add_subdirectory(
	"container"
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
