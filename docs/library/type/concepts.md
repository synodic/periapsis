---
title: Concepts
description: Type constraints for containers, algebraic types, and utilities
---

# Concepts

Periapsis provides comprehensive C++20 concepts for constraining templates, particularly around container requirements and algebraic structures.

## Utility Concepts

Basic utility concepts used throughout the library.

### decayed

Type is same as its decayed form:

```cpp
namespace peri {
    template<typename T>
    concept decayed = std::same_as<T, std::decay_t<T>>;
}
```

### invocable_r / callable

Function is invocable with return type R:

```cpp
namespace peri {
    template<typename Fn, typename R, typename... Args>
    concept invocable_r = std::is_invocable_r_v<R, Fn, Args...>;

    template<typename Fn, typename R, typename... Args>
    concept callable = invocable_r<Fn, R, Args...>;
}
```

### hash_function

Satisfies the Hash named requirement:

```cpp
namespace peri {
    template<typename Fn, typename KeyType>
    concept hash_function = /* callable with KeyType returning size_t */;
}
```

---

## Algebraic Concepts

Concepts for algebraic structures in `peri::concepts`:

### semigroup

A type with an associative binary operation:

```cpp
namespace peri {
    template<typename T>
    concept semigroup = requires(T a, T b) {
        { a + b } -> std::same_as<T>;
    };
}
```

Used by: `Fenwick` tree

### monoid

A semigroup with an identity element:

```cpp
namespace peri {
    template<typename T>
    concept monoid = semigroup<T> && requires {
        { T{} } -> std::same_as<T>;  // Identity element
    };
}
```

---

## Container Concepts

Concepts implementing the C++ named requirements for containers.

### Base Container

```cpp
namespace peri {
    // The standard Container named requirement
    template<typename T>
    concept container = /* see requirements */;

    // Container of a specific value type
    template<typename T, typename ValueType>
    concept container_of = container<T> &&
        std::same_as<ValueType, typename T::value_type>;

    // Container with mutable iterator reference
    template<typename T>
    concept mutable_container = container<T> &&
        std::same_as<std::iter_reference_t<typename T::iterator>,
                     typename T::reference>;

    // Container with size() member
    template<typename T>
    concept sized_container = container<T> && /* has size() */;

    // Container with clear() member
    template<typename T>
    concept clearable_container = container<T> && /* has clear() */;

    // Container with bidirectional iterators
    template<typename T>
    concept reversible_container = container<T> && /* has rbegin/rend */;
}
```

### Sequence Containers

```cpp
namespace peri {
    // The SequenceContainer named requirement
    template<typename T>
    concept sequence_container = container<T> && /* has front() */;

    // Double-ended access (front + back)
    template<typename T>
    concept double_ended_container = sequence_container<T> &&
        sized_container<T> && reversible_container<T> && /* has back() */;

    // Random access via operator[]
    template<typename T>
    concept random_access_container = double_ended_container<T> &&
        /* has [] and at() */;

    // Contiguous memory storage
    template<typename T>
    concept contiguous_container = random_access_container<T> &&
        /* has data() */;

    // Resizable with insert/erase
    template<typename T>
    concept resizable_sequence_container = double_ended_container<T> &&
        /* has insert, erase */;

    // Supports emplace() at arbitrary position
    template<typename T>
    concept inplace_constructing_sequence_container =
        resizable_sequence_container<T> && /* has emplace */;

    // Efficient front insertion
    template<typename T>
    concept front_growable_container = sequence_container<T> &&
        /* has push_front, pop_front */;

    // Efficient back insertion
    template<typename T>
    concept back_growable_container = double_ended_container<T> &&
        /* has push_back, pop_back */;

    // Front emplacement
    template<typename T>
    concept inplace_front_constructing_container =
        front_growable_container<T> && /* has emplace_front */;

    // Back emplacement
    template<typename T>
    concept inplace_back_constructing_container =
        back_growable_container<T> && /* has emplace_back */;
}
```

### Associative Containers

```cpp
namespace peri {
    // The AssociativeContainer named requirement
    template<typename T>
    concept associative_container = container<T> &&
        /* has key_type, find, count, etc. */;

    // No duplicate keys (set, map)
    template<typename T>
    concept unique_associative_container = associative_container<T> &&
        /* ... */;

    // Allows duplicate keys (multiset, multimap)
    template<typename T>
    concept multiple_associative_container = associative_container<T> &&
        /* ... */;

    // Ordered by key comparison
    template<typename T>
    concept ordered_associative_container = associative_container<T> &&
        /* has key_compare */;

    // Hash-based
    template<typename T>
    concept unordered_associative_container = associative_container<T> &&
        /* has hash_function, key_equal */;

    // Has mapped_type (map, multimap)
    template<typename T>
    concept map_container = associative_container<T> &&
        /* has mapped_type */;

    // Unique map (map, unordered_map)
    template<typename T>
    concept unique_map_container = map_container<T> &&
        unique_associative_container<T>;
}
```

---

## Concept Hierarchy

```
container
├── sized_container
├── clearable_container
├── mutable_container
├── reversible_container
└── sequence_container
    └── double_ended_container
        ├── random_access_container
        │   └── contiguous_container
        ├── resizable_sequence_container
        │   └── inplace_constructing_sequence_container
        ├── front_growable_container
        │   └── inplace_front_constructing_container
        └── back_growable_container
            └── inplace_back_constructing_container

container
└── associative_container
    ├── unique_associative_container
    ├── multiple_associative_container
    ├── ordered_associative_container
    ├── unordered_associative_container
    └── map_container
        └── unique_map_container
```

---

## Usage Example

```cpp
import synodic.periapsis;

using namespace peri;

// Generic function constrained by concept
template<back_growable_container C>
void append_all(C& container, std::ranges::input_range auto&& range) {
    for (auto&& elem : range) {
        container.push_back(std::forward<decltype(elem)>(elem));
    }
}

// Works with any back-growable container
std::vector<int> vec;
append_all(vec, std::array{1, 2, 3});

FixedRingBuffer<int, 64> buf;
append_all(buf, std::array{1, 2, 3});
```
