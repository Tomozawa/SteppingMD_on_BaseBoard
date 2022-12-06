#pragma once

#include <CRSLib/include/std_int.hpp>
#include <CRSLib/include/utility.hpp>

#include <CRSLib/Can/RM0008/include/hal_can.hpp>
#include <CRSLib/Can/RM0008/include/utility.hpp>

namespace CRSLib::Can::Config
{
	// フィルタバンクの個数
	inline constexpr u32 filter_bank_total_size = 14;
}
