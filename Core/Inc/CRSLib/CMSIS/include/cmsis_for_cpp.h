#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// LL/SC (もしかしたらdeprecatedかも？surpressする方法とか公式っぽいサイトで紹介されてるし、stack overflowの人たちがめっちゃ推してたので使うことにした)
uint32_t stew_load_link_32(uint32_t * src);
uint32_t stew_store_conditional_32(uint32_t val, uint32_t * dst);

// interrupt enable/disable
void stew_enable_irq(void);
void stew_disable_irq(void);

#ifdef __cplusplus
}
#endif
