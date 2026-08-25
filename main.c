#include "timer.h"
#include "gpio.h"
#include "uart.h"
#include "stm32f4xx.h"
#include <stdio.h>

volatile uint32_t timer_ticks = 0;
volatile uint8_t timer_event = 0;

int main(void)
{
    uint8_t led_state = 0;
    char buf[64];
    
    /* Enable clocks */
    RCC->AHB1ENR |= (1 << 0);   /* GPIOA */
    RCC->APB1ENR |= (1 << 0) | (1 << 17);   /* TIM2 and USART2 */

    /* Configure PA5 as output (LED) */
    GPIO_Config led_config = {
        .mode = GPIO_MODE_OUTPUT,
        .otype = GPIO_OTYPE_PP,
        .speed = GPIO_SPEED_LOW,
        .pupd = GPIO_PUPD_NONE
    };
    GPIO_Init(GPIOA, 5, &led_config);
    
    uart_init(115200);
    timer_init();
    
    uart_send_string("Timer interrupt demo started\r\n");
    
    while (1)
    {
        /* Check if timer event occurred */
        if (timer_event)
        {
            timer_event = 0;
            
            /* Toggle LED */
            GPIO_TogglePin(GPIOA, 5);
            led_state = !led_state;
            
            /* Print status via UART */
            sprintf(buf, "[%lu ms] LED: %s\r\n", 
                    timer_ticks * 100, 
                    led_state ? "ON" : "OFF");
            uart_send_string(buf);
        }
    }
}

void TIM2_IRQHandler(void)
{
    /* Check if update interrupt flag is set */
    if (TIM2->SR & (1 << 0))
    {
        /* Clear the interrupt flag */
        TIM2->SR &= ~(1 << 0);
        
        /* Increment tick counter */
        timer_ticks++;
        
        /* Set event flag for main loop */
        timer_event = 1;
    }
}