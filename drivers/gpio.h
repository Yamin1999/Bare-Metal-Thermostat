
#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "stm32f401xe.h"  // Replace with your MCU's header file

// Pin modes
typedef enum {
    GPIO_MODE_INPUT  = 0,
    GPIO_MODE_OUTPUT = 1,
    GPIO_MODE_AF     = 2,
    GPIO_MODE_ANALOG = 3
} GPIO_Mode;

// Output types
typedef enum {
    GPIO_OTYPE_PP = 0,  // Push-pull
    GPIO_OTYPE_OD = 1   // Open-drain
} GPIO_OutputType;

// Output speeds
typedef enum {
    GPIO_SPEED_LOW    = 0,
    GPIO_SPEED_MEDIUM = 1,
    GPIO_SPEED_HIGH   = 2,
    GPIO_SPEED_VHIGH  = 3
} GPIO_Speed;

// Pull-up/pull-down
typedef enum {
    GPIO_PUPD_NONE = 0,
    GPIO_PUPD_UP   = 1,
    GPIO_PUPD_DOWN = 2
} GPIO_PullUpDown;

// Pin states
typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET   = 1
} GPIO_PinState;

// Configuration structure
typedef struct {
    GPIO_Mode mode;
    GPIO_OutputType otype;
    GPIO_Speed speed;
    GPIO_PullUpDown pupd;
} GPIO_Config;

// Function prototypes
void GPIO_Init(GPIO_TypeDef *GPIOx, uint8_t pin, GPIO_Config *config);
void GPIO_WritePin(GPIO_TypeDef *GPIOx, uint8_t pin, GPIO_PinState state);
GPIO_PinState GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint8_t pin);
void GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint8_t pin);

#endif // GPIO_H