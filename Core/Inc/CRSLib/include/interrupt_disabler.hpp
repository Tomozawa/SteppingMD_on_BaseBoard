// 危険。どこかで割り込み禁止にして無限ループしてしまったりすれば、割り込みを用いた様々な機能(緊急停止など含む)が動かなくなる。
// そもそも割り込みってたくさん貯められるものなのか？同時に起こった割り込みの実行を待つやつらはどこに記憶されているんだ？

#pragma once

#include <CRSLib/CMSIS/include/cmsis_for_cpp.h>

namespace CRSLib
{
	// これを使うような処理はごくごく短いものにとどめること.
	// 安易に手を出すな.(自戒)
	class InterruptDisabler final
	{
	public:
		InterruptDisabler() noexcept
		{
			stew_disable_irq();
		}

		InterruptDisabler(const InterruptDisabler&) = delete;
		InterruptDisabler(InterruptDisabler&&) = delete;
		InterruptDisabler& operator=(const InterruptDisabler&) = delete;
		InterruptDisabler& operator=(InterruptDisabler&&) = delete;

		~InterruptDisabler()
		{
			stew_enable_irq();
		}
	};
}
