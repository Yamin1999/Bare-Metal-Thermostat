#include "uart.h"
#include "stm32f4xx.h"

void uart_init(uint32_t baud)
{
    /* PA2: alternate function mode, AF7 (USART2_TX) */
    GPIOA->MODER  &= ~(3 << 4);
    GPIOA->MODER  |=  (2 << 4);
    GPIOA->AFR[0] |=  (7 << 8);

    /* PA3: alternate function mode, AF7 (USART2_RX) */
    GPIOA->MODER  &= ~(3 << 6);
    GPIOA->MODER  |=  (2 << 6);
    GPIOA->AFR[0] |=  (7 << 12);

    /* Baud rate: USARTDIV = fCK / (16 × baud) */
    uint32_t usartdiv = (SystemCoreClock * 100) / (16 * baud);  /* × 100 for fixed-point */
    uint32_t mantissa = usartdiv / 100;                         /* Integer part */
    uint32_t fraction = ((usartdiv % 100) * 16 + 50) / 100;     /* Decimal × 16, rounded */
    USART2->BRR   = (mantissa << 4) | fraction;                 /* [15:4] | [3:0] */

    /* Enable transmitter, receiver and USART */
    USART2->CR1 |= (1 << 3);   /* TE */
    USART2->CR1 |= (1 << 2);   /* RE */
    USART2->CR1 |= (1 << 13);  /* UE */
}

void uart_send_byte(uint8_t byte)
{
    while (!(USART2->SR & (1 << 7)));  /* wait TXE */
    USART2->DR = byte;
}

uint8_t uart_receive_byte(void)
{
    while (!(USART2->SR & (1 << 5)));  /* wait RXNE */
    return (uint8_t)USART2->DR;
}

void uart_send_string(const char *str)
{
    while (*str)
    {
        uart_send_byte((uint8_t)*str++);
    }
}