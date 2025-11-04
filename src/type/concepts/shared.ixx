// LEGAL: Container concepts inspired by https://github.com/MiSo1289/more_concepts which has the following license:

/* MIT License

Copyright (c) 2021 Michal Jankovič

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

export module synodic.library:type.concepts.shared;

import std;

export namespace synodic
{
	// Verifies that the given type is the same as its decayed type
	template<typename T>
	concept decayed = std::same_as<T, std::decay_t<T>>;

	// Implements the missing concept for 'is_invocable_r'
	template<typename Fn, typename R, typename... Args>
	concept invocable_r = std::is_invocable_r_v<R, Fn, Args...>;

	// An alias for 'invocable_r'
	template<typename Fn, typename R, typename... Args>
	concept callable = invocable_r<Fn, R, Args...>;

	// A concept implementation of the named requirement 'Hash' with Standard naming styles
	//		This definition is devoid of backwards compatibility requirements
	//		https://en.cppreference.com/w/cpp/named_req/Hash
	template<typename Fn, typename KeyType>
	concept hash_function = callable<const Fn, auto(KeyType&)->std::size_t, auto(const KeyType&)->std::size_t>;
}
