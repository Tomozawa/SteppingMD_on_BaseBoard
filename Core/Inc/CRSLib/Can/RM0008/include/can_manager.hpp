#pragma once

#include <CRSLib/Can/CommonAmongMpu/include/utility.hpp>

#include "hal_can.hpp"
#include "pillarbox.hpp"
#include "letterbox.hpp"

namespace CRSLib::Can::RM0008
{
	struct CanManager final
	{
		Pillarbox pillarbox;
		Letterbox letterbox0;
		Letterbox letterbox1;

		CanManager(CAN_HandleTypeDef *const hcan) noexcept:
			pillarbox{hcan},
			letterbox0{hcan, FifoIndex::fifo0},
			letterbox1{hcan, FifoIndex::fifo1}
		{}
	};
}