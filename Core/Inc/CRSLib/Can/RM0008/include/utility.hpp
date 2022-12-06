#pragma once

namespace CRSLib::Can::RM0008
{
	enum class CanX
	{
		can1,
		can2
	};

	enum class FifoIndex : u32
	{
		fifo0 = CAN_RX_FIFO0,
		fifo1 = CAN_RX_FIFO1
	};

	enum class Bitrate : u32
	{
		rate10k = 10000,
		rate20k = 20000,
		rate50k = 50000,
		rate100k = 100000,
		rate125k = 125000,
		rate250k = 250000,
		rate500k = 500000,
		rate1M = 1000000
	};

	// 定義してね
	inline constexpr i_ptr can_instance(CanX can_x) noexcept
	{
		switch(can_x)
		{
		case CanX::can1:
			return static_cast<i_ptr>(0x4000'6400);
		case CanX::can2:
			return static_cast<i_ptr>(0x4000'6800);
		}
		return 0;
	}
}
