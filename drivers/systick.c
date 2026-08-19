#include "systick.h"
#include "stm32f4xx.h" /* Already includes core_cm4.h (SysTick pointer) + SystemCoreClock */

static volatile uint32_t tick_ms = 0;

void SysTick_Handler(void)
{
    tick_ms++;
}

void systick_init(void)
{
    SysTick->LOAD = (SystemCoreClock / 1000) - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = (1 << 2) | (1 << 1) | (1 << 0); /* CLKSOURCE, TICKINT, ENABLE */
}

void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        while (!(SysTick->CTRL & (1 << 16))); /* wait for COUNTFLAG */
    }
}

uint32_t systick_get_ticks(void)
{
    return tick_ms;
}