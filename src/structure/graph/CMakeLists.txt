target_sources(periapsis
    PUBLIC
    FILE_SET CXX_MODULES
    FILES
    "concepts.ixx"
    "storage_traits.ixx"
    "graph.ixx"
    "visitor.ixx"
)

add_subdirectory(
    storage
)
