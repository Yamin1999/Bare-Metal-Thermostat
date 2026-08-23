#include "bme280.h"
#include "uart.h"
#include "systick.h"
#include "stm32f4xx.h"
#include <stdio.h>

int main(void)
{
    /* Enable clocks */
    RCC->AHB1ENR |= ((1 << 0) | (1 << 1));  /* GPIOA and GPIOB */
    RCC->APB1ENR |= ((1 << 17) | (1 << 21)); /* USART2 and I2C1 */

    systick_init();
    uart_init(115200);
    bme280_init_i2c();

    uart_send_string("BME280 temperature monitor ready\r\n");

    while (1)
    {
        int32_t temp = bme280_read_temp_i2c();
        uint32_t pressure = bme280_read_pressure_i2c();
        uint32_t humidity = bme280_read_humidity_i2c();

        char buf[32];

        sprintf(buf, "%ld.%02ld", temp / 100, temp % 100);
        uart_send_string("Temperature: ");
        uart_send_string(buf);
        uart_send_string(" C\r\n");

        sprintf(buf, "%lu.%02lu", pressure / 25600, (pressure % 25600) / 256);
        uart_send_string("Pressure: ");
        uart_send_string(buf);
        uart_send_string(" hPa\r\n");

        sprintf(buf, "%lu.%02lu", humidity / 1024, ((humidity % 1024) * 100) / 1024);
        uart_send_string("Humidity: ");
        uart_send_string(buf);
        uart_send_string(" %RH\r\n\r\n");

        delay_ms(1000);
    }
}