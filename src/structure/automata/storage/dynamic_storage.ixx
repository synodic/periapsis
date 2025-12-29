export module synodic.periapsis:structure.automata.storage.dynamic_storage;

import :structure.automata.concepts;
import :structure.automata.storage.static_storage;

import std;

export namespace peri::automata::storage
{
	/**
	 * @brief Dynamic state machine storage supporting runtime state/transition modification.
	 *        Uses adjacency list layout for efficient traversal with full mutability.
	 *
	 * @tparam Event Type used to trigger transitions
	 * @tparam StateData Type stored per state, use void for none
	 * @tparam TransitionData Type stored per transition beyond the event, use void for none
	 * @tparam StateContainer Container template for state storage
	 * @tparam TransitionContainer Container template for per-state transition lists
	 * @tparam IndexType Unsigned integer type for descriptors
	 */
	template<
		typename Event,
		typename StateData,
		typename TransitionData,
		template<typename...> typename StateContainer = std::vector,
		template<typename...> typename TransitionContainer = std::vector,
		std::unsigned_integral IndexType = std::size_t>
	class DynamicStorage
	{
	public:
		// ========================================================================
		// Type Definitions
		// ========================================================================

		using EventType              = Event;
		using StateDescriptor        = IndexType;
		using StatesSizeType         = IndexType;
		using TransitionsSizeType    = IndexType;
		using DegreeSizeType         = IndexType;
		using StatePropertyType      = StateProperty<StateData>;
		using TransitionPropertyType = TransitionProperty<Event, TransitionData>;


		struct TransitionDescriptor
		{
			StateDescriptor source;
			IndexType transitionIndex;

			constexpr bool operator==(const TransitionDescriptor&) const noexcept  = default;
			constexpr auto operator<=>(const TransitionDescriptor&) const noexcept = default;
		};

		// Compile-time properties
		static constexpr bool HAS_STABLE_STATE_DESCRIPTORS      = false; // Invalidated on remove
		static constexpr bool HAS_STABLE_TRANSITION_DESCRIPTORS = false;

	private:
		// Transition storage: target state + property
		struct TransitionData_
		{
			StateDescriptor target;
			TransitionPropertyType property;
		};

		// Per-state data: property + out-transition list
		struct StateData_
		{
			StatePropertyType property;
			TransitionContainer<TransitionData_> outTransitions;
		};

		StateContainer<StateData_> states_;
		TransitionsSizeType totalTransitions_{0};
		std::optional<StateDescriptor> initialState_{};

	public:
		// ========================================================================
		// Construction
		// ========================================================================

		DynamicStorage() = default;

		explicit DynamicStorage(StatesSizeType initialCapacity)
		{
			states_.reserve(initialCapacity);
		}

		// ========================================================================
		// State Operations
		// ========================================================================

		[[nodiscard]] auto NumStates() const noexcept -> StatesSizeType
		{
			return static_cast<StatesSizeType>(states_.size());
		}

		[[nodiscard]] auto States() const noexcept
		{
			return std::views::iota(StateDescriptor{0}, NumStates());
		}

		auto AddState() -> StateDescriptor
		{
			auto s = static_cast<StateDescriptor>(states_.size());
			states_.emplace_back();
			return s;
		}

		template<typename... Args>
			requires std::constructible_from<StateData, Args...>
		auto AddState(Args&&... args) -> StateDescriptor
		{
			auto s             = static_cast<StateDescriptor>(states_.size());
			auto& data         = states_.emplace_back();
			data.property.data = StateData(std::forward<Args>(args)...);
			return s;
		}

		void RemoveState(StateDescriptor s)
		{
			// Remove all transitions from this state
			totalTransitions_ -= static_cast<TransitionsSizeType>(states_[s].outTransitions.size());
			states_[s].outTransitions.clear();

			// Remove all transitions targeting this state from other states
			for (auto& stateData: states_)
			{
				std::erase_if(
					stateData.outTransitions,
					[s, this](const TransitionData_& t)
					{
						if (t.target == s)
						{
							--totalTransitions_;
							return true;
						}
						return false;
					});
			}

			// Handle initial state
			if (initialState_ && *initialState_ == s)
			{
				initialState_ = std::nullopt;
			}

			// Swap with last and pop since stable state descriptors is false
			if (s != states_.size() - 1)
			{
				auto last = static_cast<StateDescriptor>(states_.size() - 1);

				// Update all transitions pointing to last to point to s
				for (auto& stateData: states_)
				{
					for (auto& trans: stateData.outTransitions)
					{
						if (trans.target == last)
						{
							trans.target = s;
						}
					}
				}

				// Update initial state if it was last
				if (initialState_ && *initialState_ == last)
				{
					initialState_ = s;
				}

				states_[s] = std::move(states_[last]);
			}
			states_.pop_back();
		}

		void ClearStates()
		{
			states_.clear();
			totalTransitions_ = 0;
			initialState_     = std::nullopt;
		}

		// ========================================================================
		// Transition Operations
		// ========================================================================

		[[nodiscard]] auto NumTransitions() const noexcept -> TransitionsSizeType
		{
			return totalTransitions_;
		}

		auto AddTransition(
			StateDescriptor source,
			StateDescriptor target,
			const Event& event)
			-> std::pair<TransitionDescriptor, bool>
		{
			auto transIndex = static_cast<IndexType>(states_[source].outTransitions.size());
			states_[source].outTransitions.push_back(TransitionData_{target, TransitionPropertyType{event}});

			++totalTransitions_;
			return {
				TransitionDescriptor{source, transIndex},
				true
			};
		}

		template<typename... Args>
			requires std::constructible_from<TransitionData, Args...>
		auto AddTransition(
			StateDescriptor source,
			StateDescriptor target,
			const Event& event,
			Args&&... args)
			-> std::pair<TransitionDescriptor, bool>
		{
			auto transIndex = static_cast<IndexType>(states_[source].outTransitions.size());
			auto prop       = TransitionPropertyType{event};
			prop.data       = TransitionData(std::forward<Args>(args)...);
			states_[source].outTransitions.push_back(TransitionData_{target, std::move(prop)});

			++totalTransitions_;
			return {
				TransitionDescriptor{source, transIndex},
				true
			};
		}

		void RemoveTransition(TransitionDescriptor t)
		{
			auto& out = states_[t.source].outTransitions;
			if (t.transitionIndex < out.size())
			{
				out.erase(out.begin() + t.transitionIndex);
				--totalTransitions_;
			}
		}

		void RemoveTransition(StateDescriptor source, StateDescriptor target, const Event& event)
		{
			auto& out = states_[source].outTransitions;
			auto it   = std::ranges::find_if(
				out,
				[target, &event](const TransitionData_& t)
				{
					return t.target == target && t.property.event == event;
				});

			if (it != out.end())
			{
				out.erase(it);
				--totalTransitions_;
			}
		}

		void ClearTransitions()
		{
			for (auto& s: states_)
			{
				s.outTransitions.clear();
			}
			totalTransitions_ = 0;
		}

		// ========================================================================
		// Transition Enumeration
		// ========================================================================

		class TransitionIterator
		{
			const DynamicStorage* storage_;
			StateDescriptor currentState_;
			IndexType currentTrans_;

			void AdvanceToValid()
			{
				while (currentState_ < storage_->NumStates())
				{
					if (currentTrans_ < storage_->states_[currentState_].outTransitions.size())
					{
						return;
					}
					++currentState_;
					currentTrans_ = 0;
				}
			}

		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type        = TransitionDescriptor;
			using difference_type   = std::ptrdiff_t;
			using pointer           = const TransitionDescriptor*;
			using reference         = TransitionDescriptor;

			TransitionIterator() = default;

			TransitionIterator(const DynamicStorage* s, StateDescriptor state, IndexType trans) :
				storage_(s),
				currentState_(state),
				currentTrans_(trans)
			{
				AdvanceToValid();
			}

			auto operator*() const -> TransitionDescriptor
			{
				return {currentState_, currentTrans_};
			}

			auto operator++() -> TransitionIterator&
			{
				++currentTrans_;
				AdvanceToValid();
				return *this;
			}

			auto operator++(int) -> TransitionIterator
			{
				auto tmp = *this;
				++*this;
				return tmp;
			}

			bool operator==(const TransitionIterator& other) const
			{
				return currentState_ == other.currentState_ && currentTrans_ == other.currentTrans_;
			}
		};

		[[nodiscard]] auto Transitions() const noexcept
		{
			return std::ranges::subrange(
				TransitionIterator{this, StateDescriptor{0}, IndexType{0}},
				TransitionIterator{this, NumStates(), IndexType{0}});
		}

		// ========================================================================
		// Incidence: Out-Transition Traversal
		// ========================================================================

		[[nodiscard]] auto OutDegree(StateDescriptor s) const noexcept -> DegreeSizeType
		{
			return static_cast<DegreeSizeType>(states_[s].outTransitions.size());
		}

		[[nodiscard]] auto OutTransitions(StateDescriptor s) const noexcept
		{
			return std::views::iota(IndexType{0}, static_cast<IndexType>(states_[s].outTransitions.size())) |
			       std::views::transform(
				       [s](IndexType i)
				       {
					       return TransitionDescriptor{s, i};
				       });
		}

		[[nodiscard]] auto Source(TransitionDescriptor t) const noexcept -> StateDescriptor
		{
			return t.source;
		}

		[[nodiscard]] auto Target(TransitionDescriptor t) const noexcept -> StateDescriptor
		{
			return states_[t.source].outTransitions[t.transitionIndex].target;
		}

		// ========================================================================
		// Event-Based Transition Lookup
		// ========================================================================

		[[nodiscard]] auto FindTransition(
			StateDescriptor s,
			const Event& event) const noexcept
			-> std::optional<TransitionDescriptor>
		{
			const auto& out = states_[s].outTransitions;
			for (IndexType i = 0; i < out.size(); ++i)
			{
				if (out[i].property.event == event)
				{
					return TransitionDescriptor{s, i};
				}
			}
			return std::nullopt;
		}

		// ========================================================================
		// Property Access
		// ========================================================================

		[[nodiscard]] auto operator[](StateDescriptor s) -> StatePropertyType&
		{
			return states_[s].property;
		}

		[[nodiscard]] auto operator[](StateDescriptor s) const -> const StatePropertyType&
		{
			return states_[s].property;
		}

		[[nodiscard]] auto operator[](TransitionDescriptor t) -> TransitionPropertyType&
		{
			return states_[t.source].outTransitions[t.transitionIndex].property;
		}

		[[nodiscard]] auto operator[](TransitionDescriptor t) const -> const TransitionPropertyType&
		{
			return states_[t.source].outTransitions[t.transitionIndex].property;
		}

		// ========================================================================
		// Initial and Final State Management
		// ========================================================================

		void SetInitialState(StateDescriptor s)
		{
			if (initialState_)
			{
				states_[*initialState_].property.isInitial = false;
			}
			initialState_                  = s;
			states_[s].property.isInitial = true;
		}

		[[nodiscard]] auto InitialState() const noexcept -> std::optional<StateDescriptor>
		{
			return initialState_;
		}

		void SetFinalState(StateDescriptor s, bool isFinal = true)
		{
			states_[s].property.isFinal = isFinal;
		}

		[[nodiscard]] auto IsFinalState(StateDescriptor s) const noexcept -> bool
		{
			return states_[s].property.isFinal;
		}

		[[nodiscard]] auto FinalStates() const noexcept
		{
			return States() | std::views::filter(
				       [this](StateDescriptor s)
				       {
					       return states_[s].property.isFinal;
				       });
		}

		// ========================================================================
		// State Data Access (convenience methods)
		// ========================================================================

		template<typename S = StateData>
			requires(not std::same_as<S, void>)
		[[nodiscard]] auto GetStateData(StateDescriptor s) -> S&
		{
			return states_[s].property.data;
		}

		template<typename S = StateData>
			requires(not std::same_as<S, void>)
		[[nodiscard]] auto GetStateData(StateDescriptor s) const -> const S&
		{
			return states_[s].property.data;
		}

		[[nodiscard]] auto TransitionEvent(TransitionDescriptor t) const noexcept -> const Event&
		{
			return states_[t.source].outTransitions[t.transitionIndex].property.event;
		}

		template<typename T = TransitionData>
			requires(not std::same_as<T, void>)
		[[nodiscard]] auto GetTransitionData(TransitionDescriptor t) -> T&
		{
			return states_[t.source].outTransitions[t.transitionIndex].property.data;
		}

		template<typename T = TransitionData>
			requires(not std::same_as<T, void>)
		[[nodiscard]] auto GetTransitionData(TransitionDescriptor t) const -> const T&
		{
			return states_[t.source].outTransitions[t.transitionIndex].property.data;
		}
	};

	// ============================================================================
	// Convenience Aliases
	// ============================================================================

	/**
	 * @brief Dynamic storage with integral events and no additional data
	 */
	template<std::unsigned_integral IndexType = std::size_t>
	using DynamicTopology = DynamicStorage<int, void, void, std::vector, std::vector, IndexType>;

	/**
	 * @brief Dynamic storage with typed events and state data
	 */
	template<typename Event, typename StateData, std::unsigned_integral IndexType = std::size_t>
	using DynamicStateStorage = DynamicStorage<Event, StateData, void, std::vector, std::vector, IndexType>;

	/**
	 * @brief Dynamic storage with typed events and transition data
	 */
	template<typename Event, typename TransitionData, std::unsigned_integral IndexType = std::size_t>
	using DynamicTransitionStorage = DynamicStorage<Event, void, TransitionData, std::vector, std::vector, IndexType>;
}
