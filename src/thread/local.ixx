export module synodic.library:thread.local;

import std;

export namespace synodic
{
	// A managed object that exists statically for each thread
	template<typename T>
	class ThreadLocal
	{
	public:
		ThreadLocal();
		~ThreadLocal();

		ThreadLocal(const ThreadLocal&)		  = delete;
		ThreadLocal(ThreadLocal&& o) noexcept = default;

		ThreadLocal& operator=(const ThreadLocal&)			 = delete;
		ThreadLocal& operator=(ThreadLocal&& other) noexcept = default;

		ThreadLocal<T>(const T&);
	};
}
