#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void    spi_init(void);
void    spi_send(uint8_t byte);
uint8_t spi_transfer(uint8_t byte);
void    spi_cs_low(void);
void    spi_cs_high(void);

#endif