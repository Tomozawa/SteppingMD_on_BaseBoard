#pragma once

namespace CRSLib
{
	inline constexpr void clear_bit(volatile auto& bit, const auto reg)
	{
		bit = bit & ~reg;
	}

	inline constexpr void clear_bit(auto& bit, const auto reg)
	{
		bit &= ~reg;
	}

	inline constexpr void set_bit(volatile auto& bit, const auto reg)
	{
		bit = bit | reg;
	}

	inline constexpr void set_bit(auto& bit, const auto reg)
	{
		bit |= reg;
	}
}
