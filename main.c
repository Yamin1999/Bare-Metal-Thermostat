#include "adc.h"
#include "gpio.h"
#include "stm32f4xx.h"

volatile uint16_t adc_result = 0;

int main(void)
{
    /* Enable GPIOA and GPIOC clock */
    RCC->AHB1ENR |= ((1 << 0) | (1 << 2));

    /* Enable ADC1 clock */
    RCC->APB2ENR |= (1 << 8);

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
    
    while(1)
    {
        adc_result = adc_read();

        if (adc_result > 2047)
        {
            GPIO_WritePin(GPIOA, 5, 1);
        }
        else
        {
            GPIO_WritePin(GPIOA, 5, 0);
        }
    }
}