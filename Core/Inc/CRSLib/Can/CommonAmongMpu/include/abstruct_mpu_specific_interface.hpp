#pragma once

#include <concepts>
#include "utility.hpp"

namespace CRSLib::Can
{
	template<class T>
	concept TxFrameC = requires(T tx_frame)
	{
		// ヘッダの型は問わないが名前くらいは統一する.
		// Idは含まずとも良い.
		tx_frame.header;

		tx_frame.data;
		requires std::same_as<decltype(tx_frame.data), DataField>;
	};

	template<class T, class TxFrame>
	concept PillarboxC = TxFrameC<TxFrame> && requires(T pillarbox, const u32 id, TxFrame tx_frame)
	{
		{pillarbox.post(id, tx_frame)} noexcept;
		{pillarbox.not_full()} noexcept -> std::same_as<bool>;
	};

	template<class T>
	concept RxFrameC = requires(T rx_frame)
	{
		rx_frame.header;
		{rx_frame.header.get_id()} noexcept -> std::same_as<u32>;

		rx_frame.data;
		requires std::same_as<decltype(rx_frame.data), DataField>;
	};

	template<class T, class RxFrame>
	concept LetterboxC = RxFrameC<RxFrame> && requires(T letterbox, RxFrame rx_frame)
	{
		{letterbox.receive(rx_frame)} noexcept;
		{letterbox.empty()} noexcept -> std::same_as<bool>;
	};
}
