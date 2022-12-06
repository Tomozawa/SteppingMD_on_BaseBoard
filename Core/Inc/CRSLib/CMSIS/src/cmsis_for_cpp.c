// cmsis_armcc.hより先にこいつをインクルードすること.
#include <CRSLib/CMSIS/include/cmsis_driver_include.h>

#include <CRSLib/CMSIS/include/cmsis_for_cpp.h>

uint32_t stew_load_link_32(uint32_t * src)
{
	return __LDREXW(src);
}

uint32_t stew_store_conditional_32(uint32_t val, uint32_t * dst)
{
	return __STREXW(val, dst);
}

void stew_enable_irq(void)
{
	__enable_irq();
}

void stew_disable_irq(void)
{
	__disable_irq();
}
