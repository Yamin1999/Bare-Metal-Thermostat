#include "ssd1306.h"
#include "bme280.h"
#include "adc.h"
#include "systick.h"
#include "uart.h"
#include "timer.h"
#include "gpio.h"
#include "spi.h"
#include "stm32f4xx.h"
#include <stdio.h>

enum app_mode {
    MODE_RUNNING,
    MODE_CONFIG
};

static volatile enum app_mode app_mode = MODE_RUNNING; /* main application mode, toggled by button press */
static volatile uint32_t last_press_tick = 0; /* for button debounce: timestamp of last valid press in ms */
static int heating = 0; /* previous heating state for UART log */
static int threshold = 25; /* target temperature in C */

static void display_update(int temp_int, int temp_frac,
    int hum_int, int hum_frac, uint32_t press_hpa)
{
    char buf[32];

    ssd1306_clear();

    /* Title bar */
    ssd1306_draw_filled_rect(0, 0, 128, 10, SSD1306_WHITE);
    ssd1306_draw_string(10, 1, "THERMOSTAT", SSD1306_BLACK);
    ssd1306_draw_hline(0, 12, 128, SSD1306_WHITE);

    /* Temperature */
    sprintf(buf, "Temp:  %d.%02d C", temp_int, temp_frac);
    ssd1306_draw_string(0, 16, buf, SSD1306_WHITE);

    /* Humidity */
    sprintf(buf, "Hum:   %d.%d %%", hum_int, hum_frac);
    ssd1306_draw_string(0, 26, buf, SSD1306_WHITE);

    /* Pressure */
    sprintf(buf, "Press: %lu hPa", press_hpa);
    ssd1306_draw_string(0, 36, buf, SSD1306_WHITE);

    /* Separator */
    ssd1306_draw_hline(0, 47, 128, SSD1306_WHITE);

    /* Threshold / target */
    sprintf(buf, "Target: %d C", threshold);
    ssd1306_draw_string(0, 52, buf, SSD1306_WHITE);

    /* Heating indicator */
    if (temp_int < threshold)
        ssd1306_draw_string(90, 52, "HEAT", SSD1306_WHITE);

    ssd1306_update();
}

static void hardware_init(void)
{
    /* AHB1: GPIO ports A, B, C */
    RCC->AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2);

    /* APB1: I2C1, USART2, TIM2, SPI2 */
    RCC->APB1ENR |= (1 << 21) | (1 << 17) | (1 << 0) | (1 << 14);

    /* APB2: ADC1, SYSCFG */
    RCC->APB2ENR |= (1 << 8) | (1 << 14);

    systick_init();
}

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

int main(void)
{
    hardware_init();

    /* SPI2 + OLED init */
    spi_init();
    ssd1306_init();

    ssd1306_draw_string(0, 0, "Initializing...", SSD1306_WHITE);
    ssd1306_update();

    /* USART2 on PA2/PA3: serial logging at 115200 baud */
    uart_init(115200);
    uart_send_string("\r\n=== Thermostat started ===\r\n");

    /* BME280 init: reads calibration data and sets forced mode */
    bme280_init_i2c();

    /* ADC1 on PA1 for potentiometer (threshold control) */
    adc_init();

    /* PA5 as output: built-in LED (heating indicator) */
    GPIO_Config led_cfg = {
        GPIO_MODE_OUTPUT,
        GPIO_OTYPE_PP,
        GPIO_SPEED_LOW,
        GPIO_PUPD_NONE
    };
    GPIO_Init(GPIOA, 5, &led_cfg);

    /* TIM2: 500 ms interrupt for LED blink in CONFIG mode */
    timer_init();

    /* PC13 input + EXTI: user button toggles running/config mode */
    button_interrupt_init();

    /* If we reach this point, initialization succeeded */
    ssd1306_clear();
    ssd1306_draw_string(0, 0, "Sensors OK", SSD1306_WHITE);
    ssd1306_update();
    delay_ms(500);

    /* --- Main loop --- */
    while (1) {
        /* --- RUNNING: read sensors, update display, control LED --- */
        int32_t  temp_raw  = bme280_read_temp_i2c();
        uint32_t press_raw = bme280_read_pressure_i2c();
        uint32_t hum_raw   = bme280_read_humidity_i2c();

        int temp_int  = temp_raw / 100;
        int temp_frac = (temp_raw % 100);
        if (temp_frac < 0) temp_frac = -temp_frac;

        int hum_int  = hum_raw / 1024;
        int hum_frac = ((hum_raw % 1024) * 10) / 1024;

        uint32_t press_hpa = press_raw / 256 / 100;

        display_update(temp_int, temp_frac, hum_int, hum_frac, press_hpa);

        /* LED PA5: on when temp below threshold */
        if (temp_int < threshold) {
            GPIO_WritePin(GPIOA, 5, GPIO_PIN_SET);
            if (!heating) {
                uart_send_string("Heat: ON\r\n");
                heating = 1;
            }
        } else {
            GPIO_WritePin(GPIOA, 5, GPIO_PIN_RESET);
            if (heating) {
                uart_send_string("Heat: OFF\r\n");
                heating = 0;
            }
        }

        /* Enter CONFIG mode when button pressed */
        if (app_mode == MODE_CONFIG) {
            uart_send_string("Mode: CONFIG\r\n");
            timer_start();                 /* start LED blink */

            while (app_mode == MODE_CONFIG) {
                threshold = 15 + (adc_read() * 21) / 4095;

                ssd1306_clear();
                ssd1306_draw_filled_rect(0, 0, 128, 10, SSD1306_WHITE);
                ssd1306_draw_string(18, 1, "CONFIG", SSD1306_BLACK);
                ssd1306_draw_hline(0, 12, 128, SSD1306_WHITE);

                char buf[32];
                sprintf(buf, "Target: %d C", threshold);
                ssd1306_draw_string(0, 28, buf, SSD1306_WHITE);
                ssd1306_draw_string(0, 48, "Press to confirm", SSD1306_WHITE);
                ssd1306_update();
            }

            timer_stop();                  /* stop LED blink */
            GPIO_WritePin(GPIOA, 5, GPIO_PIN_RESET); /* ensure LED off */
            {
                char log[48];
                sprintf(log, "Target: %d C\r\n", threshold);
                uart_send_string(log);
            }
            uart_send_string("Mode: RUNNING\r\n");
            continue;                      /* skip delay, update display now */
        }

        /* 1s delay split into 20ms chunks for responsive button */
        for (int i = 0; i < 50 && app_mode == MODE_RUNNING; i++)
            delay_ms(20);
    }
}

/* TIM2 IRQ: toggle LED PA5 every 500 ms (blink in CONFIG mode) */
void TIM2_IRQHandler(void)
{
    TIM2->SR &= ~(1 << 0);                /* clear UIF */
    GPIO_TogglePin(GPIOA, 5);
}

/* EXTI15_10 IRQ: handles PC13 button press with debounce */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR & (1 << 13)) {
        EXTI->PR = (1 << 13);              /* clear pending (write 1) */
        uint32_t now = systick_get_ticks();
        if (now - last_press_tick > 200) {  /* 200 ms debounce */
            last_press_tick = now;
            app_mode = (app_mode == MODE_RUNNING) ? MODE_CONFIG : MODE_RUNNING;
        }
    }
}
