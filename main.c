#include "gpio.h"
#include "systick.h"


static volatile uint32_t last_press_ms = 0;

static void button_interrupt_init(void)
{
    /* PC13 is input by default, no MODER config needed */

    /* Connect PC13 to EXTI13 */
    /* SYSCFG_EXTICR4: bits [7:4] = 0010 selects port C for EXTI13 */
    SYSCFG->EXTICR[3] |= (2 << 4);

    /* Unmask EXTI13 */
    EXTI->IMR |= (1 << 13);

    /* Trigger on falling edge */
    EXTI->FTSR |= (1 << 13);

    /* Enable in NVIC */
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, 1);
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR & (1 << 13))
    {
        if ((systick_get_ticks() - last_press_ms) >= 500) /* 500 ms debounce */
        {
            GPIO_TogglePin(GPIOA, 5);
            last_press_ms = systick_get_ticks();
        }

        EXTI->PR = (1 << 13); /* clear pending flag by writing 1 */
    }
}

int main(void)
{
    systick_init();

    /* Enable GPIOA and GPIOC clock */
    RCC->AHB1ENR |= ((1 << 0) | (1 << 2));

    /* Enable SYSCFG clock (needed to configure EXTI lines) */
    RCC->APB2ENR |= (1 << 14);

    GPIO_Config led_config = {
        .mode = GPIO_MODE_OUTPUT,
        .otype = GPIO_OTYPE_PP,
        .speed = GPIO_SPEED_LOW,
        .pupd = GPIO_PUPD_NONE
    };
    GPIO_Init(GPIOA, 5, &led_config);

    button_interrupt_init();

    uint32_t last_toggle = 0;

    while(1)
    {
        if ((systick_get_ticks() - last_toggle) >= 1000)
        {
            GPIO_TogglePin(GPIOA, 5);
            last_toggle = systick_get_ticks();
        }
    }
}