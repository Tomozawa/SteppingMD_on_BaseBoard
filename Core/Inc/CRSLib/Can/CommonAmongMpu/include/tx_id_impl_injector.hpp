#pragma once

#include <CRSLib/include/std_int.hpp>
#include "utility.hpp"
#include "offset_id.hpp"
#include "id_impl_injector_base.hpp"

namespace CRSLib::Can
{
	template<OffsetIdsEnumC auto offset_id>
	struct TxIdImplInjector;

	namespace Implement::TxIdImplInjectorImp
	{
		template<class T>
		inline constexpr bool is_tx_id_impl_injector = false;

		template<auto offset_id>
		inline constexpr bool is_tx_id_impl_injector<TxIdImplInjector<offset_id>> = true;
	}

	template<class T>
	concept TxIdImplInjectorC = Implement::TxIdImplInjectorImp::is_tx_id_impl_injector<T> && Implement::IdImplInjectorBase<T>;

	template<OffsetIdsEnumC auto offset_id>
	requires TxIdImplInjectorC<TxIdImplInjector<offset_id>>
	using TxIdImplInjectorAdaptor = TxIdImplInjector<offset_id>;
}
