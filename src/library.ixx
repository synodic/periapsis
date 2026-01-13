export module synodic.periapsis;

// Type partitions
export import :type.concepts.algebraic;
export import :type.traits;

// Container partitions
export import :container.sparse.bit_vector;
export import :container.sparse.index_vector;
export import :container.lockfree.spsc_queue;
export import :container.fixed.bitset;
export import :container.fixed.ring_buffer;
export import :container.dynamic.bitset;

// Structure partitions
export import :structure.tree.fenwick;
export import :structure.graph.concepts;
export import :structure.graph.storage_traits;
export import :structure.graph.storage.static_storage;
export import :structure.graph.storage.csr;
export import :structure.graph.storage.adjacency_list;
export import :structure.graph.storage.bidirectional_fixed;
export import :structure.graph.storage.undirected_fixed;
export import :structure.graph.storage.undirected_adjacency_list;
export import :structure.graph.graph;
export import :structure.graph.visitor;

// Automata partitions
export import :structure.automata.concepts;
export import :structure.automata.storage.static_storage;
export import :structure.automata.storage.dynamic_storage;
export import :structure.automata.state_machine;
export import :structure.automata.runner;

// Exception partitions
export import :exception;

// String partitions
export import :string.hash_string;
export import :string.inplace_string;

// Algorithm partitions
export import :algorithm.graph.max_flow;

// Math partitions
export import :math.concepts;
export import :math.vector;
export import :math.matrix;
export import :math.quaternion;
export import :math.geometry;

// Statistics partitions
export import :statistics.concepts;
export import :statistics.features;
export import :statistics.accumulator;
export import :statistics.query;

// Thread partitions
export import :thread.local;

// Utility partitions
export import :utility;
export import :utility.hash;
