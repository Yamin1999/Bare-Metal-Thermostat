#include "gpio.h"

int main(void)
{
    // Enable GPIOA clock (STM32 specific - adapt to your MCU)
    RCC->AHB1ENR |= (1 << 0);
    
    // Configure PA5 as output (built-in LED on my STM32F401RE Nucleo board)
    // Check your board schematic for the correct pin
    GPIO_Config led_config = {
        .mode = GPIO_MODE_OUTPUT,
        .otype = GPIO_OTYPE_PP,
        .speed = GPIO_SPEED_LOW,
        .pupd = GPIO_PUPD_NONE
    };
    GPIO_Init(GPIOA, 5, &led_config);
    
    // Blink LED
    while(1)
    {
        GPIO_TogglePin(GPIOA, 5);
        for(volatile int i = 0; i < 500000; i++);  // Simple delay
    }
}