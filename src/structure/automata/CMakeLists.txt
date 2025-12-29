target_sources(periapsis
    PUBLIC
    FILE_SET CXX_MODULES
    FILES
    "concepts.ixx"
    "state_machine.ixx"
    "runner.ixx"
)

add_subdirectory(
    storage
)
