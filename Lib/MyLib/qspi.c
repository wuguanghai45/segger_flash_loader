#include "qspi.h"
#include "stm32l4xx.h"
#include "w25q.h"

#define QSPI_AF10 0xAU
#define QSPI_PRESCALER 3U /* HCLK 80MHz / (3+1) = 20MHz */
#define QSPI_FSIZE_2MB 20U /* 2^(FSIZE+1) bytes -> 2MB when FSIZE=20 */

static void qspi_gpio_init(void)
{
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;

	/* PB10 CLK, PB11 NCS, PB1 IO0, PB0 IO1 */
	GPIOB->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 |
			  GPIO_MODER_MODE10 | GPIO_MODER_MODE11);
	GPIOB->MODER |= (GPIO_MODER_MODE0_1 | GPIO_MODER_MODE1_1 |
			 GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1);
	GPIOB->AFR[1] &= ~((0xFUL << 8) | (0xFUL << 12) | (0xFUL << 4) | (0xFUL << 0));
	GPIOB->AFR[1] |= (QSPI_AF10 << 8) | (QSPI_AF10 << 12);
	GPIOB->AFR[0] &= ~((0xFUL << 4) | (0xFUL << 0));
	GPIOB->AFR[0] |= (QSPI_AF10 << 4) | (QSPI_AF10 << 0);
	GPIOB->OSPEEDR |= (3U << GPIO_OSPEEDR_OSPEED10_Pos) |
			  (3U << GPIO_OSPEEDR_OSPEED11_Pos) |
			  (3U << GPIO_OSPEEDR_OSPEED0_Pos) |
			  (3U << GPIO_OSPEEDR_OSPEED1_Pos);

	/* PA6 IO3, PA7 IO2 */
	GPIOA->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
	GPIOA->MODER |= (GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);
	GPIOA->AFR[0] &= ~((0xFUL << 24) | (0xFUL << 28));
	GPIOA->AFR[0] |= (QSPI_AF10 << 24) | (QSPI_AF10 << 28);
	GPIOA->OSPEEDR |= (3U << GPIO_OSPEEDR_OSPEED6_Pos) |
			  (3U << GPIO_OSPEEDR_OSPEED7_Pos);
}

static void qspi_clear_flags(void)
{
	QUADSPI->FCR = QUADSPI_FCR_CTCF | QUADSPI_FCR_CTEF;
}

static void qspi_wait_tc(void)
{
	while ((QUADSPI->SR & QUADSPI_SR_TCF) == 0) {
	}
	qspi_clear_flags();
}

static void qspi_abort(void)
{
	if (QUADSPI->SR & QUADSPI_SR_BUSY) {
		QUADSPI->CR |= QUADSPI_CR_ABORT;
		while (QUADSPI->SR & QUADSPI_SR_BUSY) {
		}
		qspi_clear_flags();
	}
}

static uint32_t qspi_ccr_base(uint8_t cmd, uint32_t has_addr, uint32_t has_data, uint32_t read)
{
	uint32_t ccr = cmd;

	ccr |= QUADSPI_CCR_IMODE_0; /* instruction on 1 line */

	if (has_addr) {
		ccr |= QUADSPI_CCR_ADMODE_0 | QUADSPI_CCR_ADSIZE_1; /* 24-bit address, 1 line */
	}

	if (has_data) {
		ccr |= QUADSPI_CCR_DMODE_0; /* data on 1 line */
		ccr |= read ? QUADSPI_CCR_FMODE_0 : 0U; /* 01=indirect read, 00=indirect write */
	} else {
		ccr |= 0U; /* indirect write, instruction only */
	}

	return ccr;
}

void qspi_init(void)
{
	qspi_gpio_init();

	RCC->AHB3ENR |= RCC_AHB3ENR_QSPIEN;
	RCC->AHB3RSTR |= RCC_AHB3RSTR_QSPIRST;
	RCC->AHB3RSTR &= ~RCC_AHB3RSTR_QSPIRST;

	QUADSPI->CR = 0;
	QUADSPI->DCR = (QSPI_FSIZE_2MB << QUADSPI_DCR_FSIZE_Pos) |
		       (1U << QUADSPI_DCR_CSHT_Pos);
	QUADSPI->CR = (QSPI_PRESCALER << QUADSPI_CR_PRESCALER_Pos) | QUADSPI_CR_EN;
}

void qspi_cmd_only(uint8_t cmd)
{
	qspi_abort();
	QUADSPI->DLR = 0;
	QUADSPI->CCR = qspi_ccr_base(cmd, 0, 0, 0);
	qspi_wait_tc();
}

void qspi_cmd_read(uint8_t cmd, uint8_t *buf, uint32_t len)
{
	uint32_t i;

	if (len == 0) {
		return;
	}

	qspi_abort();
	QUADSPI->DLR = len - 1U;
	QUADSPI->CCR = qspi_ccr_base(cmd, 0, 1, 1);

	for (i = 0; i < len; i++) {
		while ((QUADSPI->SR & QUADSPI_SR_FLEVEL_Msk) == 0) {
		}
		buf[i] = *(__IO uint8_t *)&QUADSPI->DR;
	}
	qspi_wait_tc();
}

void qspi_cmd_addr_only(uint8_t cmd, uint32_t addr)
{
	qspi_abort();
	QUADSPI->DLR = 0;
	QUADSPI->CCR = qspi_ccr_base(cmd, 1, 0, 0);
	QUADSPI->AR = addr;
	qspi_wait_tc();
}

void qspi_cmd_addr_read(uint8_t cmd, uint32_t addr, uint8_t *buf, uint32_t len)
{
	uint32_t i;

	if (len == 0) {
		return;
	}

	qspi_abort();
	QUADSPI->DLR = len - 1U;
	QUADSPI->CCR = qspi_ccr_base(cmd, 1, 1, 1);
	QUADSPI->AR = addr;

	for (i = 0; i < len; i++) {
		while ((QUADSPI->SR & QUADSPI_SR_FLEVEL_Msk) == 0) {
		}
		buf[i] = *(__IO uint8_t *)&QUADSPI->DR;
	}
	qspi_wait_tc();
}

void qspi_cmd_addr_write(uint8_t cmd, uint32_t addr, const uint8_t *buf, uint32_t len)
{
	uint32_t i;

	if (len == 0) {
		return;
	}

	qspi_abort();
	QUADSPI->DLR = len - 1U;
	QUADSPI->CCR = qspi_ccr_base(cmd, 1, 1, 0);
	QUADSPI->AR = addr;

	for (i = 0; i < len; i++) {
		while ((QUADSPI->SR & QUADSPI_SR_FLEVEL_Msk) > (3U << QUADSPI_SR_FLEVEL_Pos)) {
		}
		*(__IO uint8_t *)&QUADSPI->DR = buf[i];
	}
	qspi_wait_tc();
}

void qspi_enter_memory_mapped(void)
{
	uint32_t ccr;

	qspi_abort();

	/* Standard read (0x03), 24-bit address, memory-mapped at 0x90000000 */
	ccr = W25X_ReadData;
	ccr |= QUADSPI_CCR_IMODE_0;
	ccr |= QUADSPI_CCR_ADMODE_0 | QUADSPI_CCR_ADSIZE_1;
	ccr |= QUADSPI_CCR_FMODE_0 | QUADSPI_CCR_FMODE_1;

	QUADSPI->DLR = 0;
	QUADSPI->CCR = ccr;
}

void qspi_exit_memory_mapped(void)
{
	qspi_abort();
}

uint8_t qspi_tx_rx_byte(uint8_t data, uint8_t read)
{
	(void)data;
	(void)read;
	return 0;
}
