export module synodic.periapsis:type.concepts.container;

import :type.concepts.shared;

import std;

export namespace peri
{
	// A concept implementation of the named requirement 'Container' with Standard naming styles
	//		This definition is devoid of backwards compatibility requirements
	//		https://en.cppreference.com/w/cpp/named_req/Container
	template<typename T>
	concept container =
		requires(T& container, const T& constContainer) {
			typename T::value_type;
			typename T::reference;
			typename T::const_reference;
			typename T::iterator;
			typename T::const_iterator;
			typename T::difference_type;
			typename T::size_type;

			requires decayed<typename T::value_type>;

			// Well-behaved containers should not rely on proxies for iterators / references.
			requires std::same_as<typename T::reference, typename T::value_type&>;
			requires std::same_as<typename T::const_reference, typename T::value_type const&>;


			requires std::forward_iterator<typename T::iterator>;
			requires std::forward_iterator<typename T::const_iterator>;
			requires std::convertible_to<typename T::iterator, typename T::const_iterator>;
			requires std::same_as<std::iter_value_t<typename T::iterator>, typename T::value_type>;
			requires std::same_as<std::iter_value_t<typename T::const_iterator>, typename T::value_type>;
			requires std::same_as<std::iter_reference_t<typename T::iterator>, typename T::reference> or
						 std::same_as<std::iter_reference_t<typename T::iterator>, typename T::const_reference>;
			requires std::same_as<std::iter_reference_t<typename T::const_iterator>, typename T::const_reference>;

			requires std::signed_integral<typename T::difference_type>;
			requires std::unsigned_integral<typename T::size_type>;

			// size_type should be able to represent all positive values of difference_type.
			requires std::in_range<typename T::size_type>(std::numeric_limits<typename T::difference_type>::max());

			// difference_type should be the same as the one defined by iterator_traits.
			requires std::same_as<
				typename T::difference_type,
				typename std::iterator_traits<typename T::iterator>::difference_type>;
			requires std::same_as<
				typename T::difference_type,
				typename std::iterator_traits<typename T::const_iterator>::difference_type>;

			requires not std::equality_comparable<typename T::value_type> or std::equality_comparable<T>;


			requires not std::movable<typename T::value_type> or std::movable<T>;
			requires not std::copyable<typename T::value_type> or std::copyable<T>;
			requires not std::semiregular<typename T::value_type> or std::semiregular<T>;
			requires not std::regular<typename T::value_type> or std::regular<T>;

			{
				container.begin()
				} -> std::same_as<typename T::iterator>;
			{
				container.end()
				} -> std::same_as<typename T::iterator>;
			{
				constContainer.begin()
				} -> std::same_as<typename T::const_iterator>;
			{
				constContainer.end()
				} -> std::same_as<typename T::const_iterator>;
			{
				container.cbegin()
				} -> std::same_as<typename T::const_iterator>;
			{
				container.cend()
				} -> std::same_as<typename T::const_iterator>;

			{
				constContainer.max_size()
				} -> std::same_as<typename T::size_type>;
			{
				constContainer.empty()
				} -> std::convertible_to<bool>;
		};

	// Convenience concept for identifying a container of a specific type
	template<typename T, typename ValueType>
	concept container_of = container<T> and std::same_as<ValueType, typename T::value_type>;

	template<typename T>
	concept mutable_container =
		container<T> and std::same_as<std::iter_reference_t<typename T::iterator>, typename T::reference>;

	// Convenience concept for identifying a mutable container of a specific type
	template<typename T, typename ValueType>
	concept mutable_container_of = container_of<T, ValueType> and mutable_container<T>;

	template<typename T>
	concept sized_container = container<T> and requires(const T& constContainer) {
												   {
													   constContainer.size()
													   } -> std::same_as<typename T::size_type>;
											   };

	// Convenience concept for identifying a sized container of a specific type
	template<typename T, typename ValueType>
	concept sized_container_of = container_of<T, ValueType> and sized_container<T>;

	template<typename T>
	concept clearable_container = container<T> and requires(T& container) { container.clear(); };

	// Convenience concept for identifying a clearable container of a specific type
	template<typename T, typename ValueType>
	concept clearable_container_of = container_of<T, ValueType> and clearable_container<T>;

	template<typename T>
	concept reversible_container =
		container<T> and
		requires(T& container, const T& constContainer) {
			requires std::bidirectional_iterator<typename T::iterator>;
			requires std::bidirectional_iterator<typename T::const_iterator>;

			typename T::reverse_iterator;
			typename T::const_reverse_iterator;
			requires std::bidirectional_iterator<typename T::reverse_iterator>;
			requires std::bidirectional_iterator<typename T::const_reverse_iterator>;
			requires std::convertible_to<typename T::reverse_iterator, typename T::const_reverse_iterator>;
			requires std::same_as<
				typename T::difference_type,
				typename std::iterator_traits<typename T::reverse_iterator>::difference_type>;
			requires std::same_as<
				typename T::difference_type,
				typename std::iterator_traits<typename T::const_reverse_iterator>::difference_type>;

			{
				container.rbegin()
				} -> std::same_as<typename T::reverse_iterator>;
			{
				container.rend()
				} -> std::same_as<typename T::reverse_iterator>;
			{
				constContainer.rbegin()
				} -> std::same_as<typename T::const_reverse_iterator>;
			{
				constContainer.rend()
				} -> std::same_as<typename T::const_reverse_iterator>;
			{
				container.crbegin()
				} -> std::same_as<typename T::const_reverse_iterator>;
			{
				container.crend()
				} -> std::same_as<typename T::const_reverse_iterator>;
		};

	// Convenience concept for identifying a reversable container of a specific type
	template<typename T, typename ValueType>
	concept reversible_container_of = container_of<T, ValueType> and reversible_container<T>;

}
