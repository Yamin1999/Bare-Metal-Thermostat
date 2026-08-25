#include "timer.h"
#include "stm32f4xx.h"

void timer_init(void)
{
    /* Configure TIM2 to generate an interrupt every 500 ms */

    /* Prescaler: 16 MHz / (15999 + 1) = 1 kHz */
    TIM2->PSC = 15999;

    /* Auto-reload: 1 kHz / (499 + 1) = 2 Hz (500 ms period) */
    TIM2->ARR = 499;

    /* Enable update interrupt */
    TIM2->DIER |= (1 << 0);  /* UIE: Update interrupt enable */

    /* Enable TIM2 interrupt in NVIC (low priority) */
    NVIC_SetPriority(TIM2_IRQn, 2);
    NVIC_EnableIRQ(TIM2_IRQn);

    /* Timer starts stopped, call timer_start() to begin */
}

void timer_start(void)
{
    TIM2->CNT = 0;
    TIM2->CR1 |= (1 << 0);   /* CEN: Counter enable */
}

void timer_stop(void) 
{
    TIM2->CR1 &= ~(1 << 0);  /* CEN: Counter disable */
}