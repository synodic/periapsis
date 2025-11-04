export module synodic.library:type.concepts.associative_container;

import :type.concepts.container;

import std;

export namespace synodic
{
	// A concept implementation of the named requirement 'AssociativeContainer' with Standard naming styles
	//		This definition is devoid of backwards compatibility requirements
	//		https://en.cppreference.com/w/cpp/named_req/AssociativeContainer
	template<typename T>
	concept associative_container =
		container<T> and sized_container<T> and clearable_container<T> and std::default_initializable<T> and
		requires(T& container, const T& const_cont) {
			typename T::key_type;

			// Searching
			requires requires(typename T::key_type const& key) {
						 {
							 const_cont.count(key)
							 } -> std::same_as<typename T::size_type>;
						 {
							 const_cont.contains(key)
							 } -> std::same_as<bool>;
						 {
							 container.find(key)
							 } -> std::same_as<typename T::iterator>;
						 {
							 const_cont.find(key)
							 } -> std::same_as<typename T::const_iterator>;
						 {
							 container.equal_range(key)
							 } -> std::same_as<std::pair<typename T::iterator, typename T::iterator>>;
						 {
							 const_cont.equal_range(key)
							 } -> std::same_as<std::pair<typename T::const_iterator, typename T::const_iterator>>;
					 };

			// Emplacement (default constructor)
			requires not std::default_initializable<typename T::value_type> or
						 requires(typename T::const_iterator const& hint) {
							 {
								 container.emplace_hint(hint)
								 } -> std::same_as<typename T::iterator>;
						 };

			// Copy-enabled operations
			requires not std::copyable<typename T::value_type> or
						 requires(
							 typename T::value_type const& value,
							 typename T::const_iterator const& hint,
							 typename T::iterator const& first,
							 typename T::iterator const& last,
							 std::initializer_list<typename T::value_type> const& init_list) {
							 // Range construction and assignment
							 T(first, last);
							 T(init_list);
							 container = init_list;

							 // Copy insertion
							 {
								 container.insert(hint, value)
								 } -> std::same_as<typename T::iterator>;

							 // Range insertion
							 container.insert(first, last);
							 container.insert(init_list);

							 // Emplacement (copy constructor)
							 {
								 container.emplace_hint(hint, value)
								 } -> std::same_as<typename T::iterator>;
						 };

			// Move-enabled operations
			requires not std::movable<typename T::value_type> or
						 requires(typename T::value_type && tmp_value, typename T::const_iterator const& hint) {
							 // Move insertion
							 {
								 container.insert(hint, std::move(tmp_value))
								 } -> std::same_as<typename T::iterator>;

							 // Emplacement (move constructor)
							 {
								 container.emplace_hint(hint, std::move(tmp_value))
								 } -> std::same_as<typename T::iterator>;
						 };

			// Erasure
			requires requires(
				typename T::const_iterator const& pos,
				typename T::const_iterator const& first,
				typename T::const_iterator const& last,
				typename T::key_type const& key) {
						 {
							 container.erase(pos)
							 } -> std::same_as<typename T::iterator>;
						 {
							 container.erase(first, last)
							 } -> std::same_as<typename T::iterator>;
						 {
							 container.erase(key)
							 } -> std::same_as<typename T::size_type>;
					 };
		};

	/// An associative container without multiple key occurrence.
	/// Satisfied by set, map, unordered_set and unordered_map.
	///
	/// Note: the container is required to support .emplace(Args...) for every constructor
	/// of the form value_type(Args...).
	template<typename T>
	concept unique_associative_container =
		associative_container<T> and requires(T& container, const T& const_cont) {
										 // Emplacement (default constructor)
										 requires not std::default_initializable<typename T::value_type> or
													  requires(typename T::const_iterator const& hint) {
														  {
															  container.emplace()
															  } -> std::same_as<std::pair<typename T::iterator, bool>>;
													  };

										 // Copy-enabled operations
										 requires not std::copyable<typename T::value_type> or
													  requires(typename T::value_type const& value) {
														  // Copy insertion
														  {
															  container.insert(value)
															  } -> std::same_as<std::pair<typename T::iterator, bool>>;

														  // Emplacement (copy constructor)
														  {
															  container.emplace(value)
															  } -> std::same_as<std::pair<typename T::iterator, bool>>;
													  };

										 // Move-enabled operations
										 requires not std::movable<typename T::value_type> or
													  requires(typename T::value_type && tmp_value) {
														  // Move insertion
														  {
															  container.insert(std::move(tmp_value))
															  } -> std::same_as<std::pair<typename T::iterator, bool>>;

														  // Emplacement (move constructor)
														  {
															  container.emplace(std::move(tmp_value))
															  } -> std::same_as<std::pair<typename T::iterator, bool>>;
													  };
									 };

	/// An associative container with multiple key occurrence.
	/// Satisfied by multiset, multimap, unordered_multiset and unordered_multimap.
	///
	/// Note: the container is required to support .emplace(Args...) for every constructor
	/// of the form value_type(Args...).
	template<typename T>
	concept multiple_associative_container =
		associative_container<T> and
		requires(T& container, const T& const_cont) {
			// Emplacement (default constructor)
			requires not std::default_initializable<typename T::value_type> or
						 requires(typename T::const_iterator const& hint) {
							 {
								 container.emplace()
								 } -> std::same_as<typename T::iterator>;
						 };

			// Copy-enabled operations
			requires not std::copyable<typename T::value_type> or requires(typename T::value_type const& value) {
																	  // Copy insertion
																	  {
																		  container.insert(value)
																		  } -> std::same_as<typename T::iterator>;

																	  // Emplacement (copy constructor)
																	  {
																		  container.emplace(value)
																		  } -> std::same_as<typename T::iterator>;
																  };

			// Move-enabled operations
			requires not std::movable<typename T::value_type> or requires(typename T::value_type && tmp_value) {
																	 // Move insertion
																	 {
																		 container.insert(std::move(tmp_value))
																		 } -> std::same_as<typename T::iterator>;

																	 // Emplacement (move constructor)
																	 {
																		 container.emplace(std::move(tmp_value))
																		 } -> std::same_as<typename T::iterator>;
																 };
		};

	/// An associative container ordered on keys.
	/// Satisfied by set, map, multiset and multimap.
	///
	/// Note: corresponds to the standard AssociativeContainer named requirement.
	template<typename T>
	concept ordered_associative_container =
		associative_container<T> and
		requires(T& container, const T& const_cont) {
			// Comparators
			typename T::key_compare;
			typename T::value_compare;
			requires std::strict_weak_order<typename T::key_compare, typename T::key_type, typename T::key_type>;
			requires std::strict_weak_order<typename T::value_compare, typename T::value_type, typename T::value_type>;

			// Ordered associative containers of ordered values should provide a lexicographical ordering
			requires not std::totally_ordered<typename T::value_type> or std::totally_ordered<T>;

			// Observers
			{
				const_cont.key_comp()
				} -> std::same_as<typename T::key_compare>;
			{
				const_cont.value_comp()
				} -> std::same_as<typename T::value_compare>;

			// Construction with comparator
			requires requires(typename T::key_compare const& key_comp) {
						 T(key_comp);

						 requires not std::copyable<typename T::value_type> or
									  requires(
										  typename T::iterator const& first,
										  typename T::iterator const& last,
										  std::initializer_list<typename T::value_type> const& init_list) {
										  T(first, last, key_comp);
										  T(init_list, key_comp);
									  };
					 };

			// Binary searching
			requires requires(typename T::key_type const& key) {
						 {
							 container.lower_bound(key)
							 } -> std::same_as<typename T::iterator>;
						 {
							 const_cont.lower_bound(key)
							 } -> std::same_as<typename T::const_iterator>;
						 {
							 container.upper_bound(key)
							 } -> std::same_as<typename T::iterator>;
						 {
							 const_cont.upper_bound(key)
							 } -> std::same_as<typename T::const_iterator>;
					 };
		};

	/// An ordered associative container without multiple key occurrence.
	/// Satisfied by set and map.
	template<typename T>
	concept ordered_unique_associative_container = unique_associative_container<T> and ordered_associative_container<T>;

	/// An ordered associative container with multiple key occurrence.
	/// Satisfied by multiset and multimap.
	template<typename T>
	concept ordered_multiple_associative_container =
		multiple_associative_container<T> and ordered_associative_container<T>;

	/// An associative container based on key hashing (hash table).
	/// Satisfied by unordered_set, unordered_map, unordered_multiset and unordered_multimap.
	///
	/// Note: corresponds to the standard UnorderedAssociativeContainer named requirement.
	template<typename T>
	concept unordered_associative_container =
		associative_container<T> and
		requires(T& container, const T& const_cont) {
			// Key hash function
			typename T::hasher;
			requires hash_function<typename T::hasher, typename T::key_type>;

			// Key equality predicate
			typename T::key_equal;
			requires std::equivalence_relation<typename T::key_equal, typename T::key_type, typename T::key_type>;

			// Observers
			{
				const_cont.hash_function()
				} -> std::same_as<typename T::hasher>;
			{
				const_cont.key_eq()
				} -> std::same_as<typename T::key_equal>;

			// Hash policy
			requires requires(float const& ml, std::size_t n) {
						 {
							 const_cont.load_factor()
							 } -> std::same_as<float>;
						 {
							 const_cont.max_load_factor()
							 } -> std::same_as<float>;
						 container.max_load_factor(ml);
						 container.rehash(n);
						 container.reserve(n);
					 };

			// Minimal bucket interface
			{
				const_cont.bucket_count()
				} -> std::same_as<std::size_t>;
			// Not requiring local_iterator etc as its not provided by many popular
			// non-std hash tables, e.g. absl::flat_hash_set and tsl::robin_map.

			// Construction with bucket count, hash function, and equality predicate
			requires requires(
				std::size_t const& bucket_count,
				typename T::hasher const& hash,
				typename T::key_equal const& equal) {
						 T(bucket_count);
						 T(bucket_count, hash);
						 T(bucket_count, hash, equal);

						 requires not std::copyable<typename T::value_type> or
									  requires(
										  typename T::iterator const& first,
										  typename T::iterator const& last,
										  std::initializer_list<typename T::value_type> const& init_list) {
										  T(first, last, bucket_count);
										  T(first, last, bucket_count, hash);
										  T(first, last, bucket_count, hash, equal);
										  T(init_list, bucket_count);
										  T(init_list, bucket_count, hash);
										  T(init_list, bucket_count, hash, equal);
									  };
					 };
		};

	/// A hash-based associative container without multiple key occurrence.
	/// Satisfied by unordered_set and unordered_map.
	template<typename T>
	concept unordered_unique_associative_container =
		unique_associative_container<T> and unordered_associative_container<T>;

	/// A hash-based associative container with multiple key occurrence.
	/// Satisfied by unordered_multiset and unordered_multimap.
	template<typename T>
	concept unordered_multiple_associative_container =
		multiple_associative_container<T> and unordered_associative_container<T>;

	/// An associative container that stores a mapped object alongside its keys.
	/// Satisfied by map, multimap, unordered_map and unordered_multimap.
	template<typename T>
	concept map_container =
		associative_container<T> and
		requires(T& container, const T& const_cont) {
			typename T::mapped_type;

			requires requires(typename T::const_iterator const& hint) {
						 requires not std::copyable<typename T::key_type> or
									  requires(typename T::key_type const& key) {
										  // Emplacement (key copy, mapped object default constructor)
										  requires not std::default_initializable<typename T::mapped_type> or
													   requires {
														   {
															   container.emplace_hint(hint, key)
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace_hint(
																   hint,
																   std::piecewise_construct,
																   std::forward_as_tuple(key),
																   std::forward_as_tuple())
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Emplacement (key copy, mapped object copy constructor)
										  requires not std::copyable<typename T::mapped_type> or
													   requires(typename T::mapped_type const& obj) {
														   {
															   container.emplace_hint(hint, key, obj)
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace_hint(
																   hint,
																   std::piecewise_construct,
																   std::forward_as_tuple(key),
																   std::forward_as_tuple(obj))
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Emplacement (key copy, mapped object move constructor)
										  requires not std::movable<typename T::mapped_type> or
													   requires(typename T::mapped_type && obj) {
														   {
															   container.emplace_hint(hint, key, std::move(obj))
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace_hint(
																   hint,
																   std::piecewise_construct,
																   std::forward_as_tuple(key),
																   std::forward_as_tuple(std::move(obj)))
															   } -> std::same_as<typename T::iterator>;
													   };
									  };

						 requires not std::movable<typename T::key_type> or
									  requires(typename T::key_type && key) {
										  // Emplacement (key move, mapped object default constructor)
										  requires not std::default_initializable<typename T::mapped_type> or
													   requires {
														   {
															   container.emplace_hint(hint, std::move(key))
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace_hint(
																   hint,
																   std::piecewise_construct,
																   std::forward_as_tuple(std::move(key)),
																   std::forward_as_tuple())
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Emplacement (key move, mapped object copy constructor)
										  requires not std::copyable<typename T::mapped_type> or
													   requires(typename T::mapped_type const& obj) {
														   {
															   container.emplace_hint(hint, std::move(key), obj)
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace_hint(
																   hint,
																   std::piecewise_construct,
																   std::forward_as_tuple(std::move(key)),
																   std::forward_as_tuple(obj))
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Emplacement (key move, mapped object move constructor)
										  requires not std::movable<typename T::mapped_type> or
													   requires(typename T::mapped_type && obj) {
														   {
															   container.emplace_hint(
																   hint,
																   std::move(key),
																   std::move(obj))
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace_hint(
																   hint,
																   std::piecewise_construct,
																   std::forward_as_tuple(std::move(key)),
																   std::forward_as_tuple(std::move(obj)))
															   } -> std::same_as<typename T::iterator>;
													   };
									  };
					 };
		};

	/// A map container without multiple key occurrence.
	/// Satisfied by map and unordered_map.
	///
	/// Note: the container is required to support
	/// .try_emplace(key_type const&, Args...)
	/// .try_emplace(key_type&&, Args...)
	/// .try_emplace(const_iterator, key_type const&, Args...)
	/// .try_emplace(const_iterator, key_type&&, Args...)
	/// for every constructor of the form mapped_type(Args...), and
	/// .insert_or_assign(key_type const&, M&&)
	/// .insert_or_assign(key_type&&, M&&)
	/// .insert_or_assign(const_iterator, key_type const&, M&&)
	/// .insert_or_assign(const_iterator, key_type&&, M&&)
	/// for every type M where std::is_assignable_v<mapped_type&, M&&> is true.
	template<typename T>
	concept unique_map_container =
		map_container<T> and
		requires(T& container, const T& const_cont) {
			// Element access
			requires requires(typename T::key_type const& key, typename T::key_type const& tmp_key) {
						 // operator[] access
						 requires not std::default_initializable<typename T::mapped_type> or
									  requires {
										  // Key copy
										  requires not std::copyable<typename T::key_type> or
													   requires {
														   {
															   container[key]
															   } -> std::same_as<typename T::mapped_type&>;
													   };

										  // Key move
										  requires not std::movable<typename T::key_type> or
													   requires {
														   {
															   container[std::move(tmp_key)]
															   } -> std::same_as<typename T::mapped_type&>;
													   };
									  };

						 // Checked access
						 {
							 container.at(key)
							 } -> std::same_as<typename T::mapped_type&>;
						 {
							 const_cont.at(key)
							 } -> std::same_as<typename T::mapped_type const&>;
					 };

			requires requires(typename T::const_iterator const& hint) {
						 // Key copy
						 requires not std::copyable<typename T::key_type> or
									  requires(typename T::key_type const& key) {
										  // Emplacement (key copy, mapped object default constructor)
										  requires not std::default_initializable<typename T::mapped_type> or
													   requires {
														   {
															   container.emplace(key)
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(key),
																   std::forward_as_tuple())
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(key)
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(hint, key)
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Mapped object copy
										  requires not std::copyable<typename T::mapped_type> or
													   requires(typename T::mapped_type const& obj) {
														   // Insertion
														   {
															   container.insert_or_assign(key, obj)
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.insert_or_assign(hint, key, obj)
															   } -> std::same_as<typename T::iterator>;

														   // Emplacement (key copy, mapped object copy constructor)
														   {
															   container.emplace(key, obj)
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(key),
																   std::forward_as_tuple(obj))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(key, obj)
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(hint, key, obj)
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Mapped object move
										  requires not std::movable<typename T::mapped_type> or
													   requires(typename T::mapped_type && obj) {
														   // Insertion
														   {
															   container.insert_or_assign(key, std::move(obj))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.insert_or_assign(hint, key, std::move(obj))
															   } -> std::same_as<typename T::iterator>;

														   // Emplacement (key copy, mapped object move constructor)
														   {
															   container.emplace(key, std::move(obj))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(key),
																   std::forward_as_tuple(std::move(obj)))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(key, std::move(obj))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(hint, key, std::move(obj))
															   } -> std::same_as<typename T::iterator>;
													   };
									  };

						 // Key move
						 requires not std::movable<typename T::key_type> or
									  requires(typename T::key_type && key) {
										  // Emplacement (key move, mapped object default constructor)
										  requires not std::default_initializable<typename T::mapped_type> or
													   requires {
														   {
															   container.emplace(std::move(key))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(std::move(key)),
																   std::forward_as_tuple())
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(std::move(key))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(hint, std::move(key))
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Mapped object copy
										  requires not std::copyable<typename T::mapped_type> or
													   requires(typename T::mapped_type const& obj) {
														   // Insertion
														   {
															   container.insert_or_assign(std::move(key), obj)
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.insert_or_assign(hint, std::move(key), obj)
															   } -> std::same_as<typename T::iterator>;

														   // Emplacement (key move, mapped object copy constructor)
														   {
															   container.emplace(std::move(key), obj)
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(std::move(key)),
																   std::forward_as_tuple(obj))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(std::move(key), obj)
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(hint, std::move(key), obj)
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Mapped object move
										  requires not std::movable<typename T::mapped_type> or
													   requires(typename T::mapped_type && obj) {
														   // Insertion
														   {
															   container.insert_or_assign(
																   std::move(key),
																   std::move(obj))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.insert_or_assign(
																   hint,
																   std::move(key),
																   std::move(obj))
															   } -> std::same_as<typename T::iterator>;

														   // Emplacement (key move, mapped object move constructor)
														   {
															   container.emplace(std::move(key), std::move(obj))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(std::move(key)),
																   std::forward_as_tuple(std::move(obj)))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(std::move(key), std::move(obj))
															   } -> std::same_as<std::pair<typename T::iterator, bool>>;
														   {
															   container.try_emplace(
																   hint,
																   std::move(key),
																   std::move(obj))
															   } -> std::same_as<typename T::iterator>;
													   };
									  };
					 };
		};

	/// A map container with multiple key occurrence.
	/// Satisfied by multimap and unordered_multimap.
	template<typename T>
	concept multiple_map_container =
		map_container<T> and
		requires(T& container, const T& const_cont) {
			requires requires(typename T::const_iterator const& hint) {
						 requires not std::copyable<typename T::key_type> or
									  requires(typename T::key_type const& key) {
										  // Emplacement (key copy, mapped object default constructor)
										  requires not std::default_initializable<typename T::mapped_type> or
													   requires {
														   {
															   container.emplace(key)
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(key),
																   std::forward_as_tuple())
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Emplacement (key copy, mapped object copy constructor)
										  requires not std::copyable<typename T::mapped_type> or
													   requires(typename T::mapped_type const& obj) {
														   {
															   container.emplace(key, obj)
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(key),
																   std::forward_as_tuple(obj))
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Emplacement (key copy, mapped object move constructor)
										  requires not std::movable<typename T::mapped_type> or
													   requires(typename T::mapped_type && obj) {
														   {
															   container.emplace(key, std::move(obj))
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(key),
																   std::forward_as_tuple(std::move(obj)))
															   } -> std::same_as<typename T::iterator>;
													   };
									  };

						 requires not std::movable<typename T::key_type> or
									  requires(typename T::key_type && key) {
										  // Emplacement (key move, mapped object default constructor)
										  requires not std::default_initializable<typename T::mapped_type> or
													   requires {
														   {
															   container.emplace(std::move(key))
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(std::move(key)),
																   std::forward_as_tuple())
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Emplacement (key copy, mapped object copy constructor)
										  requires not std::copyable<typename T::mapped_type> or
													   requires(typename T::mapped_type const& obj) {
														   {
															   container.emplace(std::move(key), obj)
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(std::move(key)),
																   std::forward_as_tuple(obj))
															   } -> std::same_as<typename T::iterator>;
													   };

										  // Emplacement (key copy, mapped object move constructor)
										  requires not std::movable<typename T::mapped_type> or
													   requires(typename T::mapped_type && obj) {
														   {
															   container.emplace(std::move(key), std::move(obj))
															   } -> std::same_as<typename T::iterator>;
														   {
															   container.emplace(
																   std::piecewise_construct,
																   std::forward_as_tuple(std::move(key)),
																   std::forward_as_tuple(std::move(obj)))
															   } -> std::same_as<typename T::iterator>;
													   };
									  };
					 };
		};

	/// An ordered map container.
	/// Satisfied by map and multimap.
	template<typename T>
	concept ordered_map_container = map_container<T> and ordered_associative_container<T>;

	/// An ordered map container without multiple key occurrence.
	/// Satisfied by map.
	template<typename T>
	concept ordered_unique_map_container = unique_map_container<T> and ordered_map_container<T>;

	/// An ordered map container with multiple key occurrence.
	/// Satisfied by multimap.
	template<typename T>
	concept ordered_multiple_map_container = multiple_map_container<T> and ordered_map_container<T>;

	/// A hash-based map container.
	/// Satisfied by unordered_map and unordered_multimap.
	template<typename T>
	concept unordered_map_container = map_container<T> and unordered_associative_container<T>;

	/// A hash-based map container without multiple key occurrence.
	/// Satisfied by unordered_map.
	template<typename T>
	concept unordered_unique_map_container = unique_map_container<T> and unordered_map_container<T>;

	/// A hash-based map container with multiple key occurrence.
	/// Satisfied by unordered_multimap.
	template<typename T>
	concept unordered_multiple_map_container = multiple_map_container<T> and unordered_map_container<T>;

	template<typename T, typename ValueType, typename KeyType = ValueType>
	concept associative_container_of =
		container_of<T, ValueType> and associative_container<T> and std::same_as<KeyType, typename T::key_type>;

	template<typename T, typename ValueType, typename KeyType = ValueType>
	concept unique_associative_container_of =
		associative_container_of<T, ValueType, KeyType> and unique_associative_container<T>;

	template<typename T, typename ValueType, typename KeyType = ValueType>
	concept multiple_associative_container_of =
		associative_container_of<T, ValueType, KeyType> and multiple_associative_container<T>;

	template<typename T, typename ValueType, typename KeyType = ValueType>
	concept ordered_associative_container_of =
		associative_container_of<T, ValueType, KeyType> and ordered_associative_container<T>;

	template<typename T, typename ValueType, typename KeyType = ValueType>
	concept ordered_unique_associative_container_of =
		unique_associative_container_of<T, ValueType, KeyType> and ordered_unique_associative_container<T>;

	template<typename T, typename ValueType, typename KeyType = ValueType>
	concept ordered_multiple_associative_container_of =
		multiple_associative_container_of<T, ValueType, KeyType> and ordered_multiple_associative_container<T>;

	template<typename T, typename ValueType, typename KeyType = ValueType>
	concept unordered_associative_container_of =
		associative_container_of<T, ValueType, KeyType> and unordered_associative_container<T>;

	template<typename T, typename ValueType, typename KeyType = ValueType>
	concept unordered_unique_associative_container_of =
		unique_associative_container_of<T, ValueType, KeyType> and unordered_unique_associative_container<T>;

	template<typename T, typename ValueType, typename KeyType = ValueType>
	concept unordered_multiple_associative_container_of =
		multiple_associative_container_of<T, ValueType, KeyType> and unordered_multiple_associative_container<T>;

	template<typename T, typename KeyType, typename MappedType>
	concept map_container_of = map_container<T> and std::same_as<KeyType, typename T::key_type> and
							   std::same_as<MappedType, typename T::mapped_type>;

	template<typename T, typename KeyType, typename MappedType>
	concept unique_map_container_of = map_container_of<T, KeyType, MappedType> and unique_map_container<T>;

	template<typename T, typename KeyType, typename MappedType>
	concept multiple_map_container_of = map_container_of<T, KeyType, MappedType> and multiple_map_container<T>;

	template<typename T, typename KeyType, typename MappedType>
	concept ordered_map_container_of = map_container_of<T, KeyType, MappedType> and ordered_map_container<T>;

	template<typename T, typename KeyType, typename MappedType>
	concept ordered_unique_map_container_of =
		unique_map_container_of<T, KeyType, MappedType> and ordered_unique_map_container<T>;

	template<typename T, typename KeyType, typename MappedType>
	concept ordered_multiple_map_container_of =
		multiple_map_container_of<T, KeyType, MappedType> and ordered_multiple_map_container<T>;

	template<typename T, typename KeyType, typename MappedType>
	concept unordered_map_container_of = map_container_of<T, KeyType, MappedType> and unordered_map_container<T>;

	template<typename T, typename KeyType, typename MappedType>
	concept unordered_unique_map_container_of =
		unique_map_container_of<T, KeyType, MappedType> and unordered_unique_map_container<T>;

	template<typename T, typename KeyType, typename MappedType>
	concept unordered_multiple_map_container_of =
		multiple_map_container_of<T, KeyType, MappedType> and unordered_multiple_map_container<T>;

}
