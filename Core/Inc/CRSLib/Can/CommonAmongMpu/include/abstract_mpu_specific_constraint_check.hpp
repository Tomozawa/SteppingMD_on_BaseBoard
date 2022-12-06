#pragma once

#include "abstruct_mpu_specific_interface.hpp"
#include <CRSLib/Can/Config/include/config.hpp>

namespace CRSLib::Can
{
	static_assert(TxFrameC<MpuSpecific::TxFrame>);
	static_assert(PillarboxC<MpuSpecific::Pillarbox, MpuSpecific::TxFrame>);
	static_assert(RxFrameC<MpuSpecific::RxFrame>);
	static_assert(LetterboxC<MpuSpecific::Letterbox, MpuSpecific::RxFrame>);
}