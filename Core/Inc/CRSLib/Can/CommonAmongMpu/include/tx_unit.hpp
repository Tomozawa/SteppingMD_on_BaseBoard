#pragma once

#include <CRSLib/include/std_int.hpp>
#include <CRSLib/include/utility.hpp>
#include <CRSLib/include/compile_for.hpp>

#include "utility.hpp"
#include "tx_id.hpp"
#include "unit_base.hpp"
#include "abstract_mpu_specific_constraint_check.hpp"

namespace CRSLib::Can::Implement
{
	namespace TxUnitImp
	{
		template<OffsetIdsEnumC OffsetIdsEnum, std::underlying_type_t<OffsetIdsEnum> ... offset_ids>
		std::tuple<TxId<static_cast<OffsetIdsEnum>(offset_ids)>...> calc_type_of_tx_id_tuple(std::integer_sequence<std::underlying_type_t<OffsetIdsEnum>, offset_ids ...>);
		
		template<OffsetIdsEnumC auto n>
		using TxIdTuple = decltype(calc_type_of_tx_id_tuple<decltype(n)>(std::make_integer_sequence<std::underlying_type_t<decltype(n)>, to_underlying(n)>()));
	}

	struct TxUnitBase : UnitBase
	{
		virtual void transmit(MpuSpecific::Pillarbox& pillarbox) noexcept = 0;
		using UnitBase::UnitBase;
	};

	template<OffsetIdsEnumC OffsetIdsEnum_>
	class TxUnit final : public TxUnitBase
	{
		using OffsetIdsEnum = OffsetIdsEnum_;
		TxUnitImp::TxIdTuple<OffsetIdsEnum::n> tx_ids{};

	public:
		TxUnit(const u32 base_id) noexcept:
			TxUnitBase{to_underlying(OffsetIdsEnum::n), base_id}
		{}

		void transmit(MpuSpecific::Pillarbox& pillarbox) noexcept override
		{
			auto for_body_par_id = [this, &pillarbox]<std::underlying_type_t<OffsetIdsEnum> offset_id, std::underlying_type_t<OffsetIdsEnum> n>(CompileForIndex<offset_id, n>) noexcept
			{
				CompileForIndex<offset_id + 1, n> ret{};

				ret.is_breaked = std::get<offset_id>(tx_ids).transmit(pillarbox, this->base_id);
				return ret;
			};

			compile_for(for_body_par_id, CompileForIndex<static_cast<std::underlying_type_t<OffsetIdsEnum>>(0), to_underlying(OffsetIdsEnum::n)>{});
		}

		void set_base_id(const u32 base_id) noexcept
		{
			this->base_id = base_id;
		}

		template<OffsetIdsEnum offset_id>
		void push(const MpuSpecific::TxFrame& tx_frame) noexcept
		{
			std::get<to_underlying(offset_id)>(tx_ids).queue.push(tx_frame);
		}

		template<OffsetIdsEnum offset_id>
		void clear() noexcept
		{
			std::get<to_underlying(offset_id)>(tx_ids).queue.clear();
		}
	};
}
