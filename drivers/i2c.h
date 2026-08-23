#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void    i2c_init(void);
void    i2c_start(void);
void    i2c_stop(void);
void    i2c_write_addr(uint8_t byte);
void    i2c_write_byte(uint8_t byte);
uint8_t i2c_read_byte(uint8_t ack);
uint8_t i2c_wait_addr(void);

#endif