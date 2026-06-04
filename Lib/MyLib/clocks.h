#ifndef CLOCKS_H
#define CLOCKS_H

/* HSI16 + PLL: M=2, N=20, R=2 -> 80 MHz SYSCLK (matches Zephyr DTS) */
#define RCC_PLL_M_DIV       2U
#define RCC_PLL_N_MUL       20U
#define RCC_PLL_R_DIV       2U
#define HSI_VALUE_HZ        16000000UL
#define SYSCLK_TARGET_HZ    80000000UL

#endif
