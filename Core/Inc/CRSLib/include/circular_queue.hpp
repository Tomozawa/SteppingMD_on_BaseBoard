#pragma once


#include <cstring>
#include <optional>

#include "std_int.hpp"


namespace CRSLib
{
	template<class Elem>
	struct SafeCircularQueueEraseN
	{
		virtual constexpr size_t size() const noexcept = 0; 
		virtual constexpr void push(const Elem& x) noexcept = 0;
		virtual constexpr std::optional<Elem> pop() noexcept = 0;
		virtual constexpr void clear() noexcept = 0;
	};

	template<class Elem, size_t n>
	class SafeCircularQueue final : SafeCircularQueueEraseN<Elem>
	{
		// バッファ.
		Elem buffer[n]{};

		size_t begin{0};
		size_t end{n};

		// proof size_t Size{0};
		// truth
		// {
		//		{push(...)} cause Size != n ? Size += 1 : none;
		//		{pop(...)} cause Size != 0 ? Size -= 1 : none;
				
		// }

	public:
		constexpr size_t size() const noexcept override
		{
			if(end == n) return 0;
			else
			{
				const auto&& expr = (n + end - begin) % n;
				return expr ? expr : n;
			}
		}

		constexpr void push(const Elem& x) noexcept override
		{
			if(end == n) end = begin;

			buffer[end] = x;
			
			if(end == n - 1) end = 0;
			else ++end;

			if(begin == end)
			{
				if(begin == n - 1) begin = 0;
				else ++begin;
			}
		}

		constexpr std::optional<Elem> pop() noexcept
		{
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

		constexpr void clear() noexcept
		{
			begin = 0;
			end = n;
		}
	};
}