#pragma once

#include <utility>
#include <tuple>

#include <CRSLib/include/circular_queue.hpp>
#include <CRSLib/include/executor.hpp>

#include "utility.hpp"
#include "rx_id_impl_injector.hpp"

namespace CRSLib::Can::Implement
{
	template<OffsetIdsEnumC auto offset_id>
	requires RxIdImplInjectorC<RxIdImplInjector<offset_id>>
	struct RxId
	{
		static_assert([]{return true;}(), "You may write wrong callback.");
	};

	template<OffsetIdsEnumC auto offset_id>
	requires RxIdImplInjectorC<RxIdImplInjector<offset_id>> && (!HasAnyCallback<RxIdImplInjector<offset_id>>)
	struct RxId<offset_id> final
	{
		using Impl = RxIdImplAdaptor<offset_id>;
		SafeCircularQueue<MpuSpecific::RxFrame, Impl::queue_size()> queue{};

		RxId() = default;
	};

	template<OffsetIdsEnumC auto offset_id>
	requires RxIdImplInjectorC<RxIdImplInjector<offset_id>> && HasMemberCallback<RxIdImplInjector<offset_id>>
	struct RxId<offset_id> final : Executable<void () noexcept>
	{
		using Impl = RxIdImplAdaptor<offset_id>;
		SafeCircularQueue<MpuSpecific::RxFrame, Impl::queue_size> queue{};
		Impl::CallbackArg * arg;

		constexpr RxId(Impl::CallbackArg *const arg) noexcept:
			arg{arg}
		{}

		bool call_once() noexcept override
		{
			if(const auto opt_rx_frame = queue.pop(); opt_rx_frame)
			{
				Impl::callback(*opt_rx_frame, arg);
				return true;
			}
			else return false;
		}

		void execute() noexcept override
		{
			call_once();
		}
	};

	template<OffsetIdsEnumC auto offset_id>
	requires RxIdImplInjectorC<RxIdImplInjector<offset_id>> && HasNonMemberCallback<RxIdImplInjector<offset_id>>
	struct RxId<offset_id> final : Executable<void () noexcept>
	{
		using Impl = RxIdImplAdaptor<offset_id>;
		SafeCircularQueue<MpuSpecific::RxFrame, Impl::queue_size> queue{};

		bool call_once() noexcept override
		{
			if(const auto opt_rx_frame = queue.pop(); opt_rx_frame)
			{
				Impl::callback(*opt_rx_frame);
				return true;
			}
			else return false;
		}

		void execute() noexcept override
		{
			call_once();
		}
	};
}
