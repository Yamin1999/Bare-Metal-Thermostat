#include "gpio.h"

void GPIO_Init(GPIO_TypeDef *GPIOx, uint8_t pin, GPIO_Config *config)
{
    // Configure mode
    GPIOx->MODER &= ~(0x3 << (pin * 2));
    GPIOx->MODER |= (config->mode << (pin * 2));
    
    // Configure output type (only if output or AF)
    if (config->mode == GPIO_MODE_OUTPUT || config->mode == GPIO_MODE_AF)
    {
        GPIOx->OTYPER &= ~(0x1 << pin);
        GPIOx->OTYPER |= (config->otype << pin);
    }
    
    // Configure speed
    GPIOx->OSPEEDR &= ~(0x3 << (pin * 2));
    GPIOx->OSPEEDR |= (config->speed << (pin * 2));
    
    // Configure pull-up/pull-down
    GPIOx->PUPDR &= ~(0x3 << (pin * 2));
    GPIOx->PUPDR |= (config->pupd << (pin * 2));
}

void GPIO_WritePin(GPIO_TypeDef *GPIOx, uint8_t pin, GPIO_PinState state)
{
    if (state == GPIO_PIN_SET)
    {
        GPIOx->BSRR = (1 << pin);           // Set pin high
    }
    else
    {
        GPIOx->BSRR = (1 << (pin + 16));    // Reset pin low
    }
}

GPIO_PinState GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint8_t pin)
{
    return (GPIOx->IDR & (1 << pin)) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint8_t pin)
{
    GPIOx->ODR ^= (1 << pin);
}