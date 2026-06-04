#ifndef QSPI_H
#define QSPI_H

#include <stdint.h>

void qspi_init(void);
void qspi_enter_memory_mapped(void);
void qspi_exit_memory_mapped(void);
uint8_t qspi_tx_rx_byte(uint8_t data, uint8_t read);
void qspi_cmd_only(uint8_t cmd);
void qspi_cmd_read(uint8_t cmd, uint8_t *buf, uint32_t len);
void qspi_cmd_addr_only(uint8_t cmd, uint32_t addr);
void qspi_cmd_addr_read(uint8_t cmd, uint32_t addr, uint8_t *buf, uint32_t len);
void qspi_cmd_addr_write(uint8_t cmd, uint32_t addr, const uint8_t *buf, uint32_t len);

#endif
