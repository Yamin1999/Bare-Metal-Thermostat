#include "adc.h"
#include "uart.h"
#include "gpio.h"
#include "stm32f4xx.h"
#include <stdio.h>

int main(void)
{
    /* Enable GPIOA and GPIOC clock */
    RCC->AHB1ENR |= ((1 << 0) | (1 << 2));

    /* Enable ADC1 clock */
    RCC->APB2ENR |= (1 << 8);

    /* Enable USART2 clock */
    RCC->APB1ENR |= (1 << 17);

    GPIO_Config led_config = {
        .mode = GPIO_MODE_OUTPUT,
        .otype = GPIO_OTYPE_PP,
        .speed = GPIO_SPEED_LOW,
        .pupd = GPIO_PUPD_NONE
    };
    GPIO_Init(GPIOA, 5, &led_config);

    GPIO_Config potentiometer_config = {
        .mode  = GPIO_MODE_ANALOG,
        .otype = GPIO_OTYPE_PP,   /* Not used in analog mode */
        .speed = GPIO_SPEED_LOW, /* Not critical for analog */
        .pupd  = GPIO_PUPD_NONE
    };
    GPIO_Init(GPIOA, 0, &potentiometer_config); /* PA0 as ADC input */

    adc_init();
    uart_init(115200);

    uart_send_string("UART ready\r\n");

    while(1)
    {
        uint16_t result = adc_read();
        char buf[8];
        sprintf(buf, "%u", result);
        uart_send_string("ADC Value: ");
        uart_send_string(buf);
        uart_send_string("\r\n");

        if (result > 2047)
        {
            GPIO_WritePin(GPIOA, 5, GPIO_PIN_SET);
        }
        else
        {
            GPIO_WritePin(GPIOA, 5, GPIO_PIN_RESET);
        }
    }
}