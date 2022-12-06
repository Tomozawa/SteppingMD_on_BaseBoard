#pragma once

#include <CRSLib/include/std_int.hpp>

#include <CRSLib/Can/CommonAmongMpu/include/utility.hpp>
#include <CRSLib/Can/CommonAmongMpu/include/transmitter.hpp>
#include <CRSLib/Can/CommonAmongMpu/include/receiver.hpp>

namespace Chibarobo2022
{
	using namespace CRSLib::IntegerTypes;
	//
	enum class SampleTx : u32
	{
		sample_tx,

		n
	};

	enum class SampleRx : u32
	{
		sample_rx,

		n
	};
}

namespace CRSLib::Can
{
	template<>
	struct TxIdImplInjector<Chibarobo2022::SampleTx::sample_tx>
	{
		static constexpr size_t queue_size() noexcept{return 10;}
	};

}

namespace Chibarobo2022
{
	inline CRSLib::Can::Transmitter<Chibarobo2022::SampleTx> transmitter{(u32)0x100};
}


namespace CRSLib::Can
{
	template<>
	struct RxIdImplInjector<Chibarobo2022::SampleRx::sample_rx>
	{
		static constexpr size_t queue_size() noexcept{return 20;}

		void callback(const MpuSpecific::RxFrame& rx_frame) noexcept
		{
			 Chibarobo2022::transmitter.template push<0, Chibarobo2022::SampleTx::sample_tx>(MpuSpecific::TxFrame{{rx_frame.header.dlc}, rx_frame.data});
		}
	};
}

namespace Chibarobo2022
{
	inline CRSLib::Can::Receiver<Chibarobo2022::SampleRx> receiver{(u32)0x101};
}
