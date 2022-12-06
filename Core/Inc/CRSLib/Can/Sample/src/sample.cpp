#include <CRSLib/include/executor.hpp>
#include <CRSLib/Can/CommonAmongMpu/include/pack.hpp>
#include <CRSLib/Can/RM0008/include/hal_can.hpp>
#include <CRSLib/Can/RM0008/include/can_manager.hpp>
#include <CRSLib/Can/RM0008/include/filter_manager.hpp>

#include "../include/sample.hpp"

namespace Chibarobo2022
{
	CRSLib::Executor<void () noexcept, 100> executor{};
}

using namespace CRSLib;
using namespace Chibarobo2022;
using namespace CRSLib::Can;
using namespace CRSLib::Can::RM0008;


extern "C"
{
	void sample_run(CAN_HandleTypeDef *const hcan) noexcept
	{
		CanManager can_manager{hcan};

		FilterManager::dynamic_initialize();

		FilterManager::ConfigFilterArg<FilterWidth::bit32, FilterMode::mask> filter_arg
		{
			{
				{
					.id = {0x101, 0x0, false, false},
					.mask = {max_std_id, max_ext_id, true, true}
				}
			},
			FifoIndex::fifo0,
			0,
			true
		};

		FilterManager::config_filter_bank(15, filter_arg);

		HAL_CAN_Start(hcan);

		// タイマ割り込みでexecutorにreceiver.receiveやtransmitter.transmitを詰め込んでwhile文内はexecutor.run_once()だけのが効率よく動くかも。
		while(true)
		{
			receiver.receive(can_manager.letterbox0, executor);
			receiver.receive(can_manager.letterbox1, executor);
			transmitter.transmit(can_manager.pillarbox);
			
			executor.run_once();
		}
	}
}
