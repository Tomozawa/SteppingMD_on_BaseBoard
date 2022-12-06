#pragma once

#include <type_traits>
#include <CRSLib/include/std_int.hpp>

namespace CRSLib
{
	template<class T>
	requires std::is_arithmetic_v<T>
	struct PidController final
	{
		T gain_p;
		T gain_i;
		T gain_d;
		size_t look_past_size;

		T past_error_total{};
		T last_error{};

		T calculate(const T current_error) const noexcept
		{
			return gain_p * current_error + gain_i * past_error_total + gain_d * (current_error - last_error);
		}

		void update(const T current_error) noexcept
		{
			past_error_total = (past_error_total * (look_past_size - 1) + current_error) / look_past_size;
			last_error = current_error;
		}
	};
}