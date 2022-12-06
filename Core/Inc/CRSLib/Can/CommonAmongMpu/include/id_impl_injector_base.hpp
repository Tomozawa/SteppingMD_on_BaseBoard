#pragma once

#include <CRSLib/include/utility.hpp>

namespace CRSLib::Can::Implement
{
	template<class T>
	concept IdImplInjectorBase = requires
	{
		requires is_constexpr([]{T::queue_size();});
	};
}
