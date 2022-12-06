#pragma once

#include <utility>
#include <tuple>
#include <type_traits>

#include <CRSLib/include/circular_queue.hpp>

#include "abstract_mpu_specific_constraint_check.hpp"
#include "tx_id_impl_injector.hpp"
#include "utility.hpp"

namespace CRSLib::Can::Implement
{
	template<OffsetIdsEnumC auto offset_id>
	struct TxId final
	{
		using Impl = TxIdImplInjectorAdaptor<offset_id>;
		SafeCircularQueue<MpuSpecific::TxFrame, Impl::queue_size()> queue{};

		// Mailboxが満杯になった(あるいはエラーが発生した)らfalse, そうでなければtrueを返す.
		bool transmit(MpuSpecific::Pillarbox& pillarbox, const u32 base_id) noexcept
		{
			while(true)
			{
				if(pillarbox.not_full())
				{
					if(auto opt_tx_frame = queue.pop(); !opt_tx_frame)
					{
						return true;
					}
					else
					{
						pillarbox.post(base_id, *opt_tx_frame);
					}
				}
				else
				{
					return false;
				}
			}
		}
	};
}
