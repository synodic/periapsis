export module synodic.library:container.queue;

import :exception.not_implemented;
import :type.concepts.container;
import :type.concepts.sequence_container;

export namespace synodic
{
	/**
	 * \brief A container adapter that closely follows the interface of std::queue with a ring buffer backend.
	 *		Queue fulfills the sequence_container constraint
	 * \tparam T The type to store
	 * \tparam Container A container fulfilling the sequence_container named requirement
	 */
	template<typename T, sequence_container_of<T> Container>
	class Queue
	{
	public:
		using container_type  = Container;
		using value_type	  = typename Container::value_type;
		using size_type		  = typename Container::size_type;
		using reference		  = typename Container::reference;
		using const_reference = typename Container::const_reference;

		Queue();
		explicit Queue(const Container& container);
		explicit Queue(Container&& container);

		~Queue() = default;
		[[nodiscard]] bool empty() const;

	private:
		size_type front_;

		Container data_;
	};

	template<typename T, sequence_container_of<T> Container>
	Queue<T, Container>::Queue() :
		Queue(Container())
	{
	}

	template<typename T, sequence_container_of<T> Container>
	Queue<T, Container>::Queue(const Container& container)
	{
		throw NotImplementedException ();
	}

	template<typename T, sequence_container_of<T> Container>
	Queue<T, Container>::Queue(Container&& container)
	{
		throw NotImplementedException ();
	}

	template<typename T, sequence_container_of<T> Container>
	bool Queue<T, Container>::empty() const
	{
		throw NotImplementedException ();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Deduction guides as specified: https://en.cppreference.com/w/cpp/container/queue/deduction_guides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Container>
	Queue(Container) -> Queue<typename Container::value_type, Container>;

	template<class Container, class Alloc>
	Queue(Container, Alloc) -> Queue<typename Container::value_type, Container>;
}
