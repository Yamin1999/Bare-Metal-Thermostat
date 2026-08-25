#include "timer.h"
#include "stm32f4xx.h"

void timer_init(void)
{
    /* Configure TIM2 to generate an interrupt every 100 ms */
    
    /* Prescaler: 16 MHz / (1599 + 1) = 10 kHz */
    TIM2->PSC = 1599;
    
    /* Auto-reload: 10 kHz / (999 + 1) = 10 Hz (100 ms period) */
    TIM2->ARR = 999;
    
    /* Enable update interrupt */
    TIM2->DIER |= (1 << 0);  /* UIE: Update interrupt enable */
    
    /* Enable TIM2 interrupt in NVIC */
    NVIC_EnableIRQ(TIM2_IRQn);
    
    /* Start the timer */
    TIM2->CR1 |= (1 << 0);  /* CEN: Counter enable */
}