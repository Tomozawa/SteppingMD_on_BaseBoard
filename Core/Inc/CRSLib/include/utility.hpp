#pragma once

#include <type_traits>

namespace CRSLib
{
	template<class Enum>
	requires std::is_enum_v<Enum>
	constexpr auto to_underlying(const Enum x) noexcept
	{
		return static_cast<std::underlying_type_t<Enum>>(x);
	}

	template<class Lambda, int=(Lambda{}(), 0)>
	constexpr bool is_constexpr(Lambda){return true;}
	constexpr bool is_constexpr(...) {return false;}
}