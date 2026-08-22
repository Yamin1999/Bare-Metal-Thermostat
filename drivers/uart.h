#ifndef UART_H
#define UART_H

#include <stdint.h>

void    uart_init(uint32_t baud);
void    uart_send_byte(uint8_t byte);
uint8_t uart_receive_byte(void);
void    uart_send_string(const char *str);

#endif