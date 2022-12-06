#pragma once

#include "interrupt_safe_circular_queue.hpp"

namespace CRSLib
{
	template<class F>
	struct Executable
	{
		static_assert([]{return false;}(), "Wrong template argument.");
	};

	template<class Ret, class ... Args>
	struct Executable<Ret (Args ...)>
	{
		static_assert([]{return false;}(), "Function must be specified noexcept.");
	};

	// 同じシグネチャを持つ別の関数を使うことができない.
	// C++は「名前は被らない」「名前が一致すれば同じもの」ということに頼りすぎではないだろうか.
	// オーバーライドする関数はどの関数をオーバーライドするのか指定できてもいい気がする.
	// で、ここまで名前に頼るなら, せめてコンパイル時にトークンを扱えるような仕組みがほしい.
	template<class Ret, class ... Args>
	struct Executable<Ret (Args...) noexcept>
	{
		using FunctionType = Ret (Args ...) noexcept;
		virtual Ret execute(Args ...) noexcept = 0;
		virtual ~Executable() = default;
	};

	template<class F, size_t queue_size>
	struct Executor final
	{
		static_assert([]{return false;}(), "Wrong template argument.");
	};

	template<size_t queue_size, class Ret, class ... Args>
	struct Executor<Ret (Args ...), queue_size> final
	{
		static_assert([]{return false;}(), "Function must be specified noexcept.");
	};

	template<size_t queue_size, class Ret, class ... Args>
	struct Executor<Ret (Args ...) noexcept, queue_size> final
	{
		using FunctionType = Ret (Args ...) noexcept;
		InterruptSafeCircularQueue<Executable<FunctionType> *, queue_size> queue{};

		Ret run_once(Args ... args) noexcept
		{
			if(auto opt_executable = queue.pop(); opt_executable)
			{
				return (*opt_executable)->execute(args ...);
			}
		}
	};
}
