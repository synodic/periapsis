# Concepts

The Synodic Template Library provides a comprehensive set of C++20 concepts for constraining templates, particularly around container requirements.

## Shared Concepts

Basic utility concepts used throughout the library.

```cpp
namespace synodic {
    // Type is same as its decayed form
    template<typename T>
    concept decayed = std::same_as<T, std::decay_t<T>>;

    // Function is invocable with return type R
    template<typename Fn, typename R, typename... Args>
    concept invocable_r = std::is_invocable_r_v<R, Fn, Args...>;

    // Alias for invocable_r
    template<typename Fn, typename R, typename... Args>
    concept callable = invocable_r<Fn, R, Args...>;

    // Satisfies the Hash named requirement
    template<typename Fn, typename KeyType>
    concept hash_function = callable<const Fn, auto(KeyType&)->std::size_t,
                                     auto(const KeyType&)->std::size_t>;
}
```

## Container Concepts

Concepts implementing the C++ named requirements for containers.

### Base Container

```cpp
namespace synodic {
    // The standard Container named requirement
    template<typename T>
    concept container = /* see requirements */;

    // Container of a specific value type
    template<typename T, typename ValueType>
    concept container_of = container<T> and
                           std::same_as<ValueType, typename T::value_type>;

    // Container with mutable iterator reference
    template<typename T>
    concept mutable_container = container<T> and
        std::same_as<std::iter_reference_t<typename T::iterator>, typename T::reference>;

    // Container with size() member
    template<typename T>
    concept sized_container = container<T> and /* has size() */;

    // Container with clear() member
    template<typename T>
    concept clearable_container = container<T> and /* has clear() */;

    // Container with bidirectional iterators and reverse iteration
    template<typename T>
    concept reversible_container = container<T> and /* has rbegin/rend */;
}
```

### Sequence Containers

```cpp
namespace synodic {
    // The SequenceContainer named requirement
    template<typename T>
    concept sequence_container = container<T> and /* has front() */;

    // Double-ended access (front + back)
    template<typename T>
    concept double_ended_container = sequence_container<T> and
                                     sized_container<T> and
                                     reversible_container<T> and /* has back() */;

    // Random access via operator[]
    template<typename T>
    concept random_access_container = double_ended_container<T> and /* has [] and at() */;

    // Contiguous memory storage
    template<typename T>
    concept contiguous_container = random_access_container<T> and /* has data() */;

    // Resizable with insert/erase
    template<typename T>
    concept resizable_sequence_container = double_ended_container<T> and /* ... */;

    // Supports emplace() at arbitrary position
    template<typename T>
    concept inplace_constructing_sequence_container = resizable_sequence_container<T> and /* ... */;

    // Efficient front insertion (push_front/pop_front)
    template<typename T>
    concept front_growable_container = sequence_container<T> and /* ... */;

    // Efficient back insertion (push_back/pop_back)
    template<typename T>
    concept back_growable_container = double_ended_container<T> and /* ... */;

    // Front emplacement support
    template<typename T>
    concept inplace_front_constructing_container = front_growable_container<T> and /* ... */;

    // Back emplacement support
    template<typename T>
    concept inplace_back_constructing_container = back_growable_container<T> and /* ... */;
}
```

### Associative Containers

```cpp
namespace synodic {
    // The AssociativeContainer named requirement
    template<typename T>
    concept associative_container = container<T> and /* has key_type, find, count, etc. */;

    // No duplicate keys (set, map, unordered_set, unordered_map)
    template<typename T>
    concept unique_associative_container = associative_container<T> and /* ... */;

    // Allows duplicate keys (multiset, multimap, etc.)
    template<typename T>
    concept multiple_associative_container = associative_container<T> and /* ... */;

    // Ordered by key comparison (set, map, multiset, multimap)
    template<typename T>
    concept ordered_associative_container = associative_container<T> and /* ... */;

    // Hash-based (unordered_*)
    template<typename T>
    concept unordered_associative_container = associative_container<T> and /* ... */;

    // Has mapped_type (map, multimap, unordered_map, etc.)
    template<typename T>
    concept map_container = associative_container<T> and /* has mapped_type */;

    // Unique map (map, unordered_map)
    template<typename T>
    concept unique_map_container = map_container<T> and /* ... */;
}
```

## Concept Hierarchy

```
container
├── mutable_container
├── sized_container
├── clearable_container
├── reversible_container
└── sequence_container
    └── double_ended_container
        └── random_access_container
            └── contiguous_container

container
└── associative_container
    ├── unique_associative_container
    ├── multiple_associative_container
    ├── ordered_associative_container
    ├── unordered_associative_container
    └── map_container
        ├── unique_map_container
        └── multiple_map_container
```

## Usage Example

```cpp
#include <synodic/library>

using namespace synodic;

// Constrain a function to work with any random access container
template<random_access_container Container>
auto sum_elements(const Container& c) {
    typename Container::value_type total{};
    for (const auto& elem : c) {
        total += elem;
    }
    return total;
}

// Works with vector, array, deque, etc.
std::vector<int> v{1, 2, 3};
auto s = sum_elements(v);  // OK

std::list<int> l{1, 2, 3};
// sum_elements(l);  // Error: list is not random_access_container
```

## Attribution

Container concepts inspired by [more_concepts](https://github.com/MiSo1289/more_concepts) by Michal Jankovič (MIT License).
