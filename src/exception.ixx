export module synodic.periapsis:exception;

import std;

export namespace peri
{
	/**
	 * @brief Thrown when a function or feature is not yet implemented
	 */
	class NotImplementedException final : public std::logic_error
	{
	public:
		NotImplementedException() :
			std::logic_error("Function is not yet implemented")
		{
		}

		explicit NotImplementedException(const std::string_view message) :
			std::logic_error(std::string(message))
		{
		}
	};

	/**
	 * @brief Thrown when an object is in an invalid state for the requested operation
	 */
	class InvalidStateException final : public std::logic_error
	{
	public:
		InvalidStateException() :
			std::logic_error("Object is in an invalid state for this operation")
		{
		}

		explicit InvalidStateException(const std::string_view message) :
			std::logic_error(std::string(message))
		{
		}
	};

	/**
	 * @brief Thrown when an operation times out
	 */
	class TimeoutException final : public std::runtime_error
	{
	public:
		TimeoutException() :
			std::runtime_error("Operation timed out")
		{
		}

		explicit TimeoutException(const std::string_view message) :
			std::runtime_error(std::string(message))
		{
		}
	};

	/**
	 * @brief Thrown when an operation is cancelled by the user or system
	 */
	class OperationCancelledException final : public std::runtime_error
	{
	public:
		OperationCancelledException() :
			std::runtime_error("Operation was cancelled")
		{
		}

		explicit OperationCancelledException(const std::string_view message) :
			std::runtime_error(std::string(message))
		{
		}
	};
}
