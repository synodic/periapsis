target_sources(periapsis
    PUBLIC
    FILE_SET CXX_MODULES
    FILES
    "static_storage.ixx"
    "bidirectional_fixed.ixx"
    "csr.ixx"
    "adjacency_list.ixx"
    "undirected_adjacency_list.ixx"
    "undirected_fixed.ixx"
)
