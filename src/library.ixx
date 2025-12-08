export module synodic.library;

// Type partitions
export import :type.concepts.shared;
export import :type.concepts.container;
export import :type.concepts.associative_container;
export import :type.concepts.sequence_container;
export import :type.traits;

// Container partitions
export import :container.sparse.bit_map;
export import :container.sparse.hash_map;
export import :container.sparse.hash_set;
export import :container.sparse.vector;
export import :container.lockfree.spsc_queue;
export import :container.fixed.ring_buffer;

// Structure partitions
export import :structure.graph.concepts;
export import :structure.graph.storage_traits;
export import :structure.graph.storage.static_storage;
export import :structure.graph.storage.csr;
export import :structure.graph.storage.adjacency_list;
export import :structure.graph.graph;
export import :structure.graph.visitor;

// State Machine partitions
export import :structure.state_machine.concepts;
export import :structure.state_machine.storage.static_storage;
export import :structure.state_machine.storage.dynamic_storage;
export import :structure.state_machine.state_machine;
export import :structure.state_machine.runner;

// Function partitions
export import :function.executor;

// Exception partitions
export import :exception.not_implemented;

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
