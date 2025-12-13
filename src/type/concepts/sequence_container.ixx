export module synodic.periapsis:type.concepts.sequence_container;

import :type.concepts.container;

export namespace peri
{
	// A concept implementation of the named requirement 'SequenceContainer' with Standard naming styles
	//		This definition is devoid of backwards compatibility requirements
	//		https://en.cppreference.com/w/cpp/named_req/SequenceContainer
	template<typename T>
	concept sequence_container =
		container<T> and requires(T& container, const T& constContainer) {
							 {
								 container.front()
								 } -> std::same_as<typename T::reference>;
							 {
								 constContainer.front()
								 } -> std::same_as<typename T::const_reference>;

							 requires not std::totally_ordered<typename T::value_type> or std::totally_ordered<T>;
						 };

	// Convenience function for identifying a SequenceContainer of a specific type
	template<typename T, typename ValueType>
	concept sequence_container_of = container_of<T, ValueType> and sequence_container<T>;

	template<typename T>
	concept double_ended_container = sequence_container<T> and sized_container<T> and reversible_container<T> and
									 requires(T& container, const T& constContainer) {
										 {
											 container.back()
											 } -> std::same_as<typename T::reference>;
										 {
											 constContainer.back()
											 } -> std::same_as<typename T::const_reference>;
									 };

	template<typename T>
	concept random_access_container =
		double_ended_container<T> and
		requires(T& container, const T& constContainer, typename T::size_type const index) {
			requires std::random_access_iterator<typename T::iterator>;
			requires std::random_access_iterator<typename T::const_iterator>;

			{
				container[index]
				} -> std::same_as<typename T::reference>;
			{
				constContainer[index]
				} -> std::same_as<typename T::const_reference>;

			{
				container.at(index)
				} -> std::same_as<typename T::reference>;
			{
				constContainer.at(index)
				} -> std::same_as<typename T::const_reference>;
		};

	// A concept implementation of the named requirement 'ContiguousContainer' with Standard naming styles
	//		This definition is devoid of backwards compatibility requirements
	//		https://en.cppreference.com/w/cpp/named_req/ContiguousContainer
	template<typename T>
	concept contiguous_container =
		random_access_container<T> and requires(T& container, const T& constContainer) {
										   typename T::pointer;
										   typename T::const_pointer;

										   requires std::contiguous_iterator<typename T::iterator>;
										   requires std::contiguous_iterator<typename T::const_iterator>;
										   requires std::convertible_to<typename T::pointer, typename T::const_pointer>;

										   {
											   container.data()
											   } -> std::same_as<typename T::pointer>;
										   {
											   constContainer.data()
											   } -> std::same_as<typename T::const_pointer>;
									   };

	template<typename T>
	concept resizable_sequence_container =
		double_ended_container<T> and clearable_container<T> and std::default_initializable<T> and
		requires(T& container, const typename T::const_iterator& position) {
			// Copy-enabled operations
			requires not std::copyable<typename T::value_type> or
						 requires(
							 typename T::size_type const& size,
							 typename T::value_type const& value,
							 typename T::iterator const& first,
							 typename T::iterator const& last,
							 std::initializer_list<typename T::value_type> const& init_list) {
							 // Copy insertion
							 {
								 container.insert(position, value)
								 } -> std::same_as<typename T::iterator>;
							 {
								 container.insert(position, first, last)
								 } -> std::same_as<typename T::iterator>;
							 {
								 container.insert(position, init_list)
								 } -> std::same_as<typename T::iterator>;

							 // Range construction and assignment
							 T(first, last);
							 T(init_list);
							 T(size, value);
							 container = init_list;
							 container.assign(first, last);
							 container.assign(init_list);
							 container.assign(size, value);

							 // Resizing
							 container.resize(size);
							 container.resize(size, value);
						 };

			// Move insertion
			requires not std::movable<typename T::value_type> or requires(typename T::value_type && value) {
																	 {
																		 container.insert(position, std::move(value))
																		 } -> std::same_as<typename T::iterator>;
																 };

			// Erasure
			requires requires(typename T::const_iterator const& first, typename T::const_iterator const& last) {
						 {
							 container.erase(position)
							 } -> std::same_as<typename T::iterator>;
						 {
							 container.erase(first, last)
							 } -> std::same_as<typename T::iterator>;
					 };
		};

	/// A resizable container that also allows in-place element construction.
	/// Satisfied by vector, deque and list.
	///
	/// Note: the container is required to support .emplace(const_iterator, Args...)
	/// for every constructor of the form value_type(Args...).
	template<typename T>
	concept inplace_constructing_sequence_container =
		resizable_sequence_container<T> and
		requires(T& container, const typename T::const_iterator& pos) {
			// Emplacement (default constructor)
			requires not std::default_initializable<typename T::value_type> or
						 requires {
							 {
								 container.emplace(pos)
								 } -> std::same_as<typename T::iterator>;
						 };

			// Emplacement (copy constructor)
			requires not std::copyable<typename T::value_type> or requires(typename T::value_type const& value) {
																	  {
																		  container.emplace(pos, value)
																		  } -> std::same_as<typename T::iterator>;
																  };

			// Emplacement (move constructor)
			requires not std::movable<typename T::value_type> or requires(typename T::value_type && value) {
																	 {
																		 container.emplace(pos, std::move(value))
																		 } -> std::same_as<typename T::iterator>;
																 };
		};

	/// A sequence container that allows efficient insertion and erasure at the front.
	/// Satisfied by deque, list and forward_list.
	template<typename T>
	concept front_growable_container =
		sequence_container<T> and std::default_initializable<T> and
		requires(T& container, const typename T::value_type& value, typename T::value_type&& tmp_value) {
			// Front copy insertion
			requires not std::copyable<typename T::value_type> or requires { container.push_front(value); };

			// Front move insertion
			requires not std::movable<typename T::value_type> or
						 requires { container.push_front(std::move(tmp_value)); };

			// Front erasure
			container.pop_front();
		};

	/// A front-growable container that also allows inplace front construction.
	/// Satisfied by deque, list and forward_list.
	///
	/// Note: the container is required to support .emplace_front(Args...)
	/// for every constructor of the form value_type(Args...).
	template<typename T>
	concept inplace_front_constructing_container =
		front_growable_container<T> and
		requires(T& container, const typename T::value_type& value, typename T::value_type&& tmp_value) {
			// Front emplacement (default constructor)
			requires not std::default_initializable<typename T::value_type> or
						 requires {
							 {
								 container.emplace_front()
								 } -> std::same_as<typename T::reference>;
						 };

			// Front emplacement (copy constructor)
			requires not std::copyable<typename T::value_type> or requires {
																	  {
																		  container.emplace_front(value)
																		  } -> std::same_as<typename T::reference>;
																  };

			// Front emplacement (move constructor)
			requires not std::movable<typename T::value_type> or requires {
																	 {
																		 container.emplace_front(std::move(tmp_value))
																		 } -> std::same_as<typename T::reference>;
																 };
		};

	/// A double-ended container that allows efficient insertion and erasure at the back.
	/// Satisfied by vector, basic_string, deque, and list.
	template<typename T>
	concept back_growable_container =
		double_ended_container<T> and std::default_initializable<T> and
		requires(T& container, const typename T::value_type& value, typename T::value_type&& tmp_value) {
			// Back copy insertion
			requires not std::copyable<typename T::value_type> or requires { container.push_back(value); };

			// Back move insertion
			requires not std::movable<typename T::value_type> or
						 requires { container.push_back(std::move(tmp_value)); };

			// Back erasure
			container.pop_back();
		};

	/// A back-growable container that also allows inplace back construction.
	/// Satisfied by vector, deque, and list.
	///
	/// Note: the container is required to support .emplace_back(Args...)
	/// for every constructor of the form value_type(Args...).
	template<typename T>
	concept inplace_back_constructing_container =
		back_growable_container<T> and
		requires(T& container, const typename T::value_type& value, typename T::value_type&& tmp_value) {
			// Back emplacement (default constructor)
			requires not std::default_initializable<typename T::value_type> or
						 requires {
							 {
								 container.emplace_back()
								 } -> std::same_as<typename T::reference>;
						 };

			// Back emplacement (copy constructor)
			requires not std::copyable<typename T::value_type> or requires {
																	  {
																		  container.emplace_back(value)
																		  } -> std::same_as<typename T::reference>;
																  };

			// Back emplacement (move constructor)
			requires not std::movable<typename T::value_type> or requires {
																	 {
																		 container.emplace_back(std::move(tmp_value))
																		 } -> std::same_as<typename T::reference>;
																 };
		};

	template<typename T, typename ValueType>
	concept double_ended_container_of = container_of<T, ValueType> and double_ended_container<T>;

	template<typename T, typename ValueType>
	concept random_access_container_of = container_of<T, ValueType> and random_access_container<T>;

	template<typename T, typename ValueType>
	concept contiguous_container_of = container_of<T, ValueType> and contiguous_container<T>;

	template<typename T, typename ValueType>
	concept resizable_sequence_container_of = container_of<T, ValueType> and resizable_sequence_container<T>;

	template<typename T, typename ValueType>
	concept inplace_constructing_sequence_container_of =
		container_of<T, ValueType> and inplace_back_constructing_container<T>;

	template<typename T, typename ValueType>
	concept front_growable_container_of = container_of<T, ValueType> and front_growable_container<T>;

	template<typename T, typename ValueType>
	concept inplace_front_constructing_container_of =
		container_of<T, ValueType> and inplace_front_constructing_container<T>;

	template<typename T, typename ValueType>
	concept back_growable_container_of = container_of<T, ValueType> and back_growable_container<T>;

	template<typename T, typename ValueType>
	concept inplace_back_constructing_container_of =
		container_of<T, ValueType> and inplace_back_constructing_container<T>;

}
