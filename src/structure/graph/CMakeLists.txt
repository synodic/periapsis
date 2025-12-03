target_sources(synodic-template-library
    PUBLIC
    FILE_SET CXX_MODULES
    FILES
    "concepts.ixx"
    "storage_traits.ixx"
    "graph.ixx"
)

add_subdirectory(
    storage
)
