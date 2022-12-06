#pragma once

#include <CRSLib/include/std_int.hpp>
#include "utility.hpp"
#include "offset_id.hpp"
#include "id_impl_injector_base.hpp"
#include "abstract_mpu_specific_constraint_check.hpp"

namespace CRSLib::Can
{
	template<OffsetIdsEnumC auto offset_id>
	class RxIdImplInjector;

	namespace Implement
	{
		template<class T>
		concept HasAnyCallback =
			requires(MpuSpecific::RxFrame rx_frame)
			{
				T::callback(rx_frame);
			} ||
			requires(MpuSpecific::RxFrame rx_frame, typename T::CallbackArg * arg)
			{
				T::callback(rx_frame, arg);
			};

		template<class T>
		concept HasNonMemberCallback = requires(MpuSpecific::RxFrame rx_frame)
		{
			{T::callback(rx_frame)} noexcept;
		};

		template<class T>
		concept HasMemberCallback = requires(MpuSpecific::RxFrame rx_frame, typename T::CallbackArg * arg)
		{
			{T::callback(rx_frame, arg)} noexcept;
		};

		namespace RxIdImplInjectorImp
		{
			template<class T>
			inline constexpr bool is_rx_id_impl_injector = false;

			template<auto offset_id>
			inline constexpr bool is_rx_id_impl_injector<RxIdImplInjector<offset_id>> = true;
		}

		template<class T>
		concept RxIdImplInjectorC = RxIdImplInjectorImp::is_rx_id_impl_injector<T> && IdImplInjectorBase<T>;

		template<OffsetIdsEnumC auto offset_id>
		requires RxIdImplInjectorC<RxIdImplInjector<offset_id>>
		using RxIdImplAdaptor = RxIdImplInjector<offset_id>;
	}
}
