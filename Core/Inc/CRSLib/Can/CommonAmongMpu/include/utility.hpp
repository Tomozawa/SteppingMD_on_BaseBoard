// For user
// nullとして予約されたidなどの便利な定数と、DataField型が定義されている。

// For me
// なるべく追加しないこと。依存性が複雑になる。

#pragma once

#include <array>

#include <CRSLib/include/std_int.hpp>

namespace CRSLib::Can
{
	// CANは最大8バイトを送受信できる。
	inline constexpr u32 can_mtu = 8;
	// もっとも大きい標準id
	inline constexpr u32 max_std_id = 0x07'FF;
	// もっとも大きい拡張id
	inline constexpr u32 max_ext_id = 0x03'FF'FF;
	// 最大のidは29bit全部1である。
	inline constexpr u32 null_id = max_ext_id << (u32)18 | max_std_id;
	// 送受信されるデータを表す型
	using DataField = std::array<u8, can_mtu>;
}
