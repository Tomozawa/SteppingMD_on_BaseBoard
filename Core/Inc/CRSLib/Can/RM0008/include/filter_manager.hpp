// なんでCAN_TypeDefのFilterIdHighとかu16じゃなくてu32なの...？
// -> あまりにあんまりなため, HAL_ConfigFilter相当の機能を自作することにした. 可搬性は同程度に維持するつもりだ.
// DualCANでも動き, 関連の薄い変更(DualCANの場合のCAN2が使うフィルタの先頭インデックスの変更と1つのフィルタの変更とか)を分離する.

#pragma once

#include <cstring>
#include <utility>
#include <algorithm>
#include <concepts>

#include <stm32f1xx.h>

#include <CRSLib/include/bit_operate.hpp>
#include <CRSLib/Can/CommonAmongMpu/include/utility.hpp>

#include "utility.hpp"
#include "filter.hpp"
#include "CRSLib/Can/RM0008/Config/include/config.hpp"

// この中の機能を使う場合には, これ以外からフィルタバンクを弄ってはならない.
namespace CRSLib::Can::RM0008::FilterManager
{
	// filer_match_indexはRM0008参照。config_filter_bankを呼び出す前に値を設定するものではなく、呼び出した後にそのフィルタのFMIが入るものだ。
	// 大抵の場合、filterとfifoだけ設定すればいい。
	template<FilterWidth filter_width, FilterMode filter_mode>
	struct ConfigFilterArg final
	{
		Filter<filter_width, filter_mode> filter;
		FifoIndex fifo{};
		u32 filter_match_index{};
		bool activate{false};
	};

	namespace Implement::FilterManagerImp
	{
		// priority: {filter_match_index[7:6] | is_can2[:5] | is_fifo1[:4] | is_bit16[:3] | is_mask[:2] : index[1:0]}
		template<FilterWidth filter_width, FilterMode filter_mode>
		constexpr u8 convert_to_priority(const u8 index, const u8 can2_start, const ConfigFilterArg<filter_width, filter_mode>& filter_arg) noexcept
		{
			return (can2_start <= index) << (u8)5 | (filter_arg.fifo == FifoIndex::fifo1) << (u8)4 | (filter_width == FilterWidth::bit16) << (u8)3 | (filter_mode == FilterMode::mask) << (u8)2 | index;
		}

		template<FilterWidth ... filter_widthes, FilterMode ... filter_modes>
		void set_filter_match_index(const u8 can2_start, ConfigFilterArg<filter_widthes, filter_modes>& ... filter_args) noexcept
		{
			std::array<u8, sizeof...(filter_widthes)> priorities;

			[can2_start, &priorities, &filter_args...]<u8 ... indices>(std::integer_sequence<u8, indices ...>) noexcept
			{
				priorities = {convert_to_priority(indices, can2_start, filter_args) ...};
			}(std::make_integer_sequence<u8, sizeof...(filter_widthes)>());
			
			std::ranges::sort(priorities);

			for(u32 can_fifo = 0b00; can_fifo <= 0b11; ++can_fifo)
			{
				for(u32 i = 0; auto& priority : priorities)
				{
					priority |= (i << 6);
					++i;

					if(!((priority & 0b11 << 4) ^ can_fifo << 4))
					{
						break;
					}
				}
			}

			std::ranges::sort(priorities, {}, [](const auto priority){return priority & 0b11;});

			[&priorities, &filter_args...]<u8 ... indices>(std::integer_sequence<u8, indices ...>) noexcept
			{
				((filter_args.filter_match_index = priorities[indices] >> 6), ...);
			}(std::make_integer_sequence<u8, sizeof...(filter_widthes)>());
		}
	}

	template<u8 index, FilterWidth filter_width, FilterMode filter_mode>
	void per_filter_arg(CAN_TypeDef *const bxcan, ConfigFilterArg<filter_width, filter_mode>& filter_arg) noexcept
	{
		constexpr u32 bit_position = (u32)1 << index;

		// change filter content.
		u32 tmp_buffer[2];
		std::memcpy(tmp_buffer, &filter_arg.filter, 8);
		bxcan->sFilterRegister[index].FR1 = tmp_buffer[0];
		bxcan->sFilterRegister[index].FR2 = tmp_buffer[1];

		// change scale
		if constexpr(filter_width == FilterWidth::bit16)
		{
			clear_bit(bxcan->FS1R, bit_position);
		}
		else
		{
			set_bit(bxcan->FS1R, bit_position);
		}

		// change mode
		if constexpr(filter_mode == FilterMode::mask)
		{
			clear_bit(bxcan->FM1R, bit_position);
		}
		else
		{
			set_bit(bxcan->FM1R, bit_position);
		}

		// change fifo
		if(filter_arg.fifo == FifoIndex::fifo0)
		{
			clear_bit(bxcan->FFA1R, bit_position);
		}
		else
		{
			set_bit(bxcan->FFA1R, bit_position);
		}

		// change active
		if(filter_arg.activate)
		{
			set_bit(bxcan->FA1R, bit_position);
		}
		else
		{
			clear_bit(bxcan->FA1R, bit_position);
		}
	}

	template<FilterWidth ... filter_widthes, FilterMode ... filter_modes>
	void config_filter_bank(const u8 can2_start, ConfigFilterArg<filter_widthes, filter_modes>& ... filter_args)
	requires (sizeof...(filter_args) <= Config::filter_bank_total_size)
	{
		// assign FMI.
		Implement::FilterManagerImp::set_filter_match_index(can2_start, filter_args ...);

		CAN_TypeDef *const bxcan1 = (CAN_TypeDef *)can_instance(CanX::can1);

		// set FINIT
		set_bit(bxcan1->FMR, CAN_FMR_FINIT);

		// change CAN2SB (in CAN1)
		clear_bit(bxcan1->FMR, CAN_FMR_CAN2SB);
		set_bit(bxcan1->FMR, can2_start << CAN_FMR_CAN2SB_Pos);

		[bxcan1]<u8 ... indices>(std::integer_sequence<u8, indices ...>, std::same_as<ConfigFilterArg<filter_widthes, filter_modes>> auto& ... filter_args)
		{
			(per_filter_arg<indices>(bxcan1, filter_args), ...);
		}(std::make_integer_sequence<u8, sizeof...(filter_widthes)>(), filter_args ...);

		// clear FINIT
		clear_bit(bxcan1->FMR, CAN_FMR_FINIT);
	}

	template<FilterWidth ... filter_widthes, FilterMode ... filter_modes>
	void config_filter_bank(ConfigFilterArg<filter_widthes, filter_modes>& ... filter_args)
	requires (sizeof...(filter_args) <= Config::filter_bank_total_size)
	{
		config_filter_bank(28, filter_args ...);
	}

	inline void change_filter_activeness(const u32 index, const bool activate) noexcept
	{
		const auto bxcan1 = (CAN_TypeDef *)can_instance(CanX::can1);
		clear_bit(bxcan1->FA1R, (u32)1 << index);
		set_bit(bxcan1->FA1R, (u32)activate << index);
	}

	inline void dynamic_initialize() noexcept
	{
		[]<u8 ... indices>(std::integer_sequence<u8, indices ...>)
		{
			constexpr FrameFeature<FilterWidth::bit32> null_feature{.std_id = max_std_id, .ext_id = max_ext_id};
			auto null_filter_arg = ConfigFilterArg<FilterWidth::bit32, FilterMode::list>{{null_feature, null_feature}, FifoIndex::fifo0};
			config_filter_bank(Config::filter_bank_total_size, static_cast<decltype(null_filter_arg)&>(static_cast<void>(indices), null_filter_arg)...);
		}(std::make_integer_sequence<u8, Config::filter_bank_total_size>());
	}
}
