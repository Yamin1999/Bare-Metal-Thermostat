#include "adc.h"
#include "stm32f4xx.h"

void adc_init(void)
{
    /* Channel 0 sampling time: 56 cycles (SMP0 = 011b, bits [2:0] of SMPR2) */
    ADC1->SMPR2 |= (3 << 0);

    /* Regular sequence: 1 conversion, channel 0 */
    ADC1->SQR1 = 0;
    ADC1->SQR3 = 0;

    /* Power on ADC */
    ADC1->CR2 |= (1 << 0);
}

uint16_t adc_read(void)
{
    /* Start a single conversion */
    ADC1->CR2 |= (1 << 30);  /* SWSTART = 1 */

    /* Wait for end of conversion */
    while (!(ADC1->SR & (1 << 1)));   /* poll EOC flag, bit 1 */

    return (uint16_t)ADC1->DR;
}