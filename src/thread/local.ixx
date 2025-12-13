export module synodic.periapsis:thread.local;

import std;

export namespace peri
{
	// A managed object that exists statically for each thread
	template<typename T>
	class ThreadLocal
	{
	public:
		ThreadLocal() = default;
		~ThreadLocal() = default;

		ThreadLocal(const ThreadLocal&)		  = delete;
		ThreadLocal(ThreadLocal&& o) noexcept = default;

		ThreadLocal& operator=(const ThreadLocal&)			 = delete;
		ThreadLocal& operator=(ThreadLocal&& other) noexcept = default;

		ThreadLocal<T>(const T&);
	};
}
