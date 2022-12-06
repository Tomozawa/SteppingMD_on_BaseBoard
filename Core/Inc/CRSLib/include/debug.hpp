#pragma once

#include <CRSLib/include/std_int.hpp>

namespace CRSLib::Debug
{
	inline constinit volatile bool error_happened{false};
	inline constinit const char *volatile error_message{nullptr};

	inline void set_error(const char *const str = "Error happened.") noexcept
	{
		error_happened = true;
		if(!error_message) error_message = str;
	}

	void error_handler() noexcept;
}
