#pragma once

#include "hal_can.hpp"

#include <CRSLib/include/std_int.hpp>
#include <CRSLib/Can/CommonAmongMpu/include/utility.hpp>

namespace CRSLib::Can::RM0008
{
	struct TxHeader final
	{
		// 送信するデータ長	(0~8)
		u8 dlc;
		// リモートフレームかどうか
		bool rtr{false};
		// Transmit Global Time機能を使うかどうか(注意: これが使えるモードに設定してないといけない)
		bool transmit_global_time{false};
	};

	struct TxFrame final
	{
		TxHeader header{};
		DataField data{};
	};

	class Pillarbox final
	{
		CAN_HandleTypeDef * hcan;

	public:
		Pillarbox(CAN_HandleTypeDef *const hcan) noexcept:
			hcan{hcan}
		{}

		void post(const u32 id, TxFrame& frame) noexcept
		{
			CAN_TxHeaderTypeDef tx_header
			{
				.StdId = id & max_std_id,
				.ExtId = id >> (u32)11,
				.IDE = (id >> (u32)11) ? CAN_ID_EXT : CAN_ID_STD,
				.RTR = frame.header.rtr ? CAN_RTR_REMOTE : CAN_RTR_DATA,
				.DLC = frame.header.dlc,
				.TransmitGlobalTime = frame.header.transmit_global_time ? ENABLE : DISABLE
			};

			u32 mailbox{};

			HAL_CAN_AddTxMessage(hcan, &tx_header, frame.data.data(), &mailbox);
		}

		bool not_full() const noexcept
		{
			return HAL_CAN_GetTxMailboxesFreeLevel(hcan) != 0;
		}
	};
}