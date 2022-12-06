#pragma once

#include <concepts>
#include <type_traits>

namespace CRSLib
{
	namespace Implement::CompileForImp
	{
		template<class T>
		 concept ForIterC = requires(T iter)
		{
			requires std::same_as<decltype(iter.is_breaked), bool>;
			requires (std::same_as<typename T::comptime_is_breaked, std::true_type> ||
			 std::same_as<typename T::comptime_is_breaked, std::false_type>);
		};
	}

	template<class BodyFunc, Implement::CompileForImp::ForIterC ForIter>
	constexpr void compile_for(BodyFunc&& body_func, ForIter&& for_iter) noexcept
	{
		auto&& next_iter = body_func(for_iter);
		if(next_iter) return;
		else if constexpr(not std::remove_cvref_t<decltype(next_iter)>::comptime_is_breaked::value)
		{
			compile_for(static_cast<BodyFunc&&>(body_func), static_cast<decltype(next_iter)&&>(next_iter));
		}
	}

	template<auto index, decltype(index) n>
	struct CompileForIndex final
	{
		bool is_breaked{false};
		constexpr explicit operator bool() const noexcept
		{
			return is_breaked;
		}
		using comptime_is_breaked = std::bool_constant<index == n>;
	};
}
