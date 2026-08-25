#ifndef BME280_H
#define BME280_H

#include <stdint.h>

#define BME280_ADDR 0x76

/* I2C functions */
void    bme280_init_i2c(void);
void    bme280_wait_ready_i2c(void);
int32_t bme280_read_temp_i2c(void);
uint32_t bme280_read_pressure_i2c(void);
uint32_t bme280_read_humidity_i2c(void);

/* SPI functions */
void     bme280_init_spi(void);
int32_t  bme280_read_temp_spi(void);
uint32_t bme280_read_pressure_spi(void);
uint32_t bme280_read_humidity_spi(void);

#endif