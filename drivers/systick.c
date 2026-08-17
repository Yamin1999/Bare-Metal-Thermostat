#include "systick.h"
#include "stm32f4xx.h" /* Already includes core_cm4.h (SysTick pointer) + SystemCoreClock */

void systick_init(void)
{
    SysTick->LOAD = (SystemCoreClock / 1000) - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = (1 << 2) | (1 << 0); /* CLKSOURCE = CPU clock, ENABLE = 1 */
}

void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        while (!(SysTick->CTRL & (1 << 16))); /* wait for COUNTFLAG */
    }
}