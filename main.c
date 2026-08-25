#include "ssd1306.h"
#include "bme280.h"
#include "adc.h"
#include "systick.h"
#include "uart.h"
#include "gpio.h"
#include "spi.h"
#include "stm32f4xx.h"
#include <stdio.h>

volatile int32_t temp_raw = 0;
volatile uint32_t press_raw = 0;
volatile uint32_t hum_raw = 0;

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

    /* If we reach this point, initialization succeeded */
    ssd1306_clear();
    ssd1306_draw_string(0, 0, "Sensors OK", SSD1306_WHITE);
    ssd1306_update();
    

    /* --- Main loop --- */
    while (1) {
        /* --- RUNNING: read sensors, update display, control LED --- */
        temp_raw  = bme280_read_temp_i2c();
        press_raw = bme280_read_pressure_i2c();
        hum_raw   = bme280_read_humidity_i2c();

        int temp_int  = temp_raw / 100;
        int temp_frac = (temp_raw % 100);
        if (temp_frac < 0) temp_frac = -temp_frac;

        int hum_int  = hum_raw / 1024;
        int hum_frac = ((hum_raw % 1024) * 10) / 1024;

        uint32_t press_hpa = press_raw / 256 / 100;

        display_update(temp_int, temp_frac, hum_int, hum_frac, press_hpa);

        delay_ms(1000);
    }
    
}