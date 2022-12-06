#pragma once

#include <utility>
#include <tuple>

#include <CRSLib/include/std_int.hpp>
#include <CRSLib/include/utility.hpp>
#include <CRSLib/include/debug.hpp>

namespace CRSLib::Can
{
	// idの範囲チェックに関する機能を有する.
	class UnitBase
	{
	public:
		const u32 id_num;
		u32 base_id;

	public:
		UnitBase(const u32 id_num, const u32 base_id) noexcept:
			id_num{id_num},
			base_id{base_id}
		{}

	public:
		bool is_in(const u32 id) const noexcept
		{
			return base_id < id && id < base_id + id_num;
		}

		friend constexpr bool is_overlap(const UnitBase& l, const UnitBase& r) noexcept
		{
			return l.base_id < r.base_id + r.id_num && r.base_id < l.base_id + l.id_num;
		}
	};
}
