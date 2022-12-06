#pragma once


#include <cstring>
#include <optional>

#include "std_int.hpp"
#include "interrupt_disabler.hpp"

namespace CRSLib
{
	template<class Elem>
	struct InterruptSafeCircularQueueEraseN
	{
		virtual void push(const Elem& x) noexcept = 0;
		virtual std::optional<Elem> pop() noexcept = 0;
		virtual void clear() noexcept = 0;
	};

	template<class Elem, size_t n>
	class InterruptSafeCircularQueue final : InterruptSafeCircularQueueEraseN<Elem>
	{
		// バッファ.
		Elem buffer[n]{};

		size_t begin{0};
		size_t end{n};

	public:
		void push(const Elem& x) noexcept override
		{
			InterruptDisabler interrupt_disabler{};
			if(end == n) end = 0;

			buffer[end] = x;
			
			if(end == n - 1) end = 0;
			else ++end;

			if(begin == end)
			{
				if(begin == n - 1) begin = 0;
				else ++begin;
			}
		}

		// 割り込み安全にpopする.
		std::optional<Elem> pop() noexcept
		{
			InterruptDisabler interrupt_disabler{};

			if(end == n)
			{
				return std::nullopt;
			}

			Elem ret = buffer[begin];

			if(begin == n - 1) begin = 0;
			else ++begin;

			if(begin == end)
			{
				end = n;
			}

			return ret;
		}

		// 割り込み安全にキューを空にする.
		void clear() noexcept
		{
			InterruptDisabler interrupt_disabler{};
			
			end = begin;
		}
	};
}