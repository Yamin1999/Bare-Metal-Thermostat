#include "adc.h"
#include "stm32f4xx.h"

void adc_init(void)
{
    /* PA1 as analog input (MODER = 11) */
    GPIOA->MODER |= (3 << 2);

    /* Channel 1 sampling time: 56 cycles (SMP1 = 011b, bits [5:3] of SMPR2) */
    ADC1->SMPR2 |= (3 << 3);

    /* Regular sequence: 1 conversion, channel 1 */
    ADC1->SQR1 = 0;
    ADC1->SQR3 = 1;

    /* Power on ADC */
    ADC1->CR2 |= (1 << 0);
}

uint16_t adc_read(void)
{
    /* Start a single conversion */
    ADC1->CR2 |= (1 << 30);   /* SWSTART = 1 */

    /* Wait for end of conversion */
    while (!(ADC1->SR & (1 << 1)));   /* poll EOC flag, bit 1 */

    return (uint16_t)ADC1->DR;
}