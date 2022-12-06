#pragma once

#include <utility>
#include <type_traits>
#include <tuple>
#include <array>
#include <algorithm>

#include <CRSLib/include/debug.hpp>

#include "offset_id.hpp"
#include "tx_unit.hpp"
#include "abstract_mpu_specific_constraint_check.hpp"

namespace CRSLib::Can
{
	template<OffsetIdsEnumC ... OffsetIdsEnums>
		requires (sizeof...(OffsetIdsEnums) > 0)
	class Transmitter final
	{
		std::tuple<Implement::TxUnit<OffsetIdsEnums> ...> tx_units{};
		
		std::array<Implement::TxUnitBase *, sizeof...(OffsetIdsEnums)> tx_units_p
		{
			[this]<size_t ... indices>(std::index_sequence<indices ...>)
			{
				return
				 std::array<Implement::TxUnitBase *, sizeof...(OffsetIdsEnums)>
				 {&std::get<indices>(tx_units) ...};
			}(std::make_index_sequence<sizeof...(OffsetIdsEnums)>())
		};

		bool is_id_sorted{false};

	public:
		Transmitter(const std::same_as<u32> auto ... args) noexcept:
			tx_units{args ...}
		{
			assert_not_overlap();
			sort_tx_unit();
		}

	public:
		void transmit(MpuSpecific::Pillarbox& pillarbox) noexcept
		{
			using namespace IntegerLiterals;

			if(not is_id_sorted)
			{
				Debug::set_error("Transmitter::transmit is called when is_id_sorted = false");
				Debug::error_handler();
			}

			for(const auto unit_p : tx_units_p)
			{
				unit_p->transmit(pillarbox);
			}
		}

		template<size_t index>
		constexpr void set_base_id(const u32 base_id) noexcept
		{
			is_id_sorted = false;
			std::get<index>(tx_units).set_base_id(base_id);
		}

		template<size_t index, OffsetIdsEnumC auto offset_id>
		void push(const MpuSpecific::TxFrame& tx_frame) noexcept
		{
			std::get<index>(tx_units).template push<offset_id>(tx_frame);
		}

		template<size_t index, OffsetIdsEnumC auto offset_id>
		void clear(const MpuSpecific::TxFrame& tx_frame) noexcept
		{
			std::get<index>(tx_units).template clear<offset_id>(tx_frame);
		}

	private:
		bool is_tx_units_not_overlap() noexcept
		{
			for(size_t i = 0; i < sizeof...(OffsetIdsEnums) - 1; ++i)
			{
				if(is_overlap(*tx_units_p[i], *tx_units_p[i + 1]))
				{
					return false;
				}
			}

			return true;
		}

		void assert_not_overlap() noexcept
		{
			if(!is_tx_units_not_overlap())
			{
				Debug::set_error("Tx Unit must not overlap.");
				Debug::error_handler();
			}
		}

		void sort_tx_unit() noexcept
		{
			std::ranges::sort(tx_units_p, {}, [](const Implement::TxUnitBase * p){return p->base_id;});
			is_id_sorted = true;
		}
	};
}
