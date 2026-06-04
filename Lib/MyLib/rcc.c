#include "rcc.h"
#include "clocks.h"

void Delay_C(uint32_t nTime)
{
	for (uint32_t i = 0; i < (SystemCoreClock / nTime); i++) {
		__asm volatile("NOP");
	}
}

void RCC_init(void)
{
	/* Enable HSI16 and switch PLL: 16MHz / 2 * 20 / 2 = 80 MHz */
	RCC->CR |= RCC_CR_HSION;
	while ((RCC->CR & RCC_CR_HSIRDY) == 0) {
	}

	FLASH->ACR = FLASH_ACR_LATENCY_4WS;

	RCC->PLLCFGR =
		RCC_PLLCFGR_PLLSRC_HSI |
		RCC_PLLCFGR_PLLREN |
		((RCC_PLL_M_DIV - 1U) << RCC_PLLCFGR_PLLM_Pos) |
		(RCC_PLL_N_MUL << RCC_PLLCFGR_PLLN_Pos) |
		(0U << RCC_PLLCFGR_PLLR_Pos); /* PLLR=0 -> divide by 2 */

	RCC->CR |= RCC_CR_PLLON;
	while ((RCC->CR & RCC_CR_PLLRDY) == 0) {
	}

	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL) {
	}

	SystemCoreClock = SYSCLK_TARGET_HZ;
	INFO("SystemCoreClock %d", SystemCoreClock);
}

void system_reset(void)
{
	SCB->AIRCR = (0x5FAUL << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk;
	__DSB();
	while (1) {
	}
}
