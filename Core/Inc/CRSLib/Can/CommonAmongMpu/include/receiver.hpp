#pragma once

#include <type_traits>
#include <tuple>

#include <CRSLib/include/compile_for.hpp>
#include <CRSLib/include/std_int.hpp>

#include "utility.hpp"
#include "offset_id.hpp"
#include "rx_unit.hpp"
#include "abstract_mpu_specific_constraint_check.hpp"
namespace CRSLib::Can
{
	namespace Implement::ReceiverImp
	{
		template<class T>
		concept ReceiverArgC = std::same_as<T, u32> || std::is_pointer_v<T>;
	}

	template<OffsetIdsEnumC ... OffsetIdsEnums>
	class Receiver final
	{
		std::tuple<Implement::RxUnit<OffsetIdsEnums> ...> rx_units;

	public:

		Receiver(const Implement::ReceiverImp::ReceiverArgC auto ... args) noexcept:
			rx_units{args ...}
		{}

		// 各rx_unitsに受信したフレームを振り分け, executorにコールバックをpushする.
		template<size_t queue_size>
		void receive(MpuSpecific::Letterbox& letterbox, Executor<void () noexcept, queue_size>& executor) noexcept
		{
			while(true)
			{
				// フレームを準備
				MpuSpecific::RxFrame rx_frame{};

				if(!letterbox.empty())
				{
					letterbox.receive(rx_frame);
				}
				else return;

				// rx_units内のrx_unitそれぞれのreceiveを呼び出す
				auto for_body_par_rx_unit = [this, &rx_frame, &executor]<size_t index, size_t n>(CompileForIndex<index, n>) noexcept
				{
					CompileForIndex<index + 1, n> ret{};

					std::get<index>(rx_units).receive(rx_frame, executor);

					return ret;
				};

				compile_for(for_body_par_rx_unit, CompileForIndex<(size_t)0, sizeof...(OffsetIdsEnums)>{});

			}
		}

		template<size_t index>
		auto& get() noexcept
		{
			return std::get<index>(rx_units);
		}
	};
}
