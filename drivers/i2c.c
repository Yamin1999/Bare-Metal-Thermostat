#include "i2c.h"
#include "stm32f4xx.h"

void i2c_init(void)
{
    /* PB8: alternate function, open-drain, AF4 (I2C1_SCL) */
    GPIOB->MODER  &= ~(3 << 16);
    GPIOB->MODER  |=  (2 << 16);
    GPIOB->OTYPER |=  (1 << 8);
    GPIOB->AFR[1] |=  (4 << 0);

    /* PB9: alternate function, open-drain, AF4 (I2C1_SDA) */
    GPIOB->MODER  &= ~(3 << 18);
    GPIOB->MODER  |=  (2 << 18);
    GPIOB->OTYPER |=  (1 << 9);
    GPIOB->AFR[1] |=  (4 << 4);

    /* Reset I2C1 */
    RCC->APB1RSTR |=  (1 << 21);
    RCC->APB1RSTR &= ~(1 << 21);

    /* Configure clock for standard mode (100 kHz) */
    /* APB1 = 16 MHz, CCR = 16000000 / (2 × 100000) = 80 */
    I2C1->CR2  |= 16;           /* FREQ = 16 MHz */
    I2C1->CCR  |= 80;           /* CCR for 100 kHz */
    I2C1->TRISE = 17;           /* TRISE = FREQ + 1 */

    /* Enable I2C peripheral */
    I2C1->CR1 |= (1 << 0);      /* PE */
}

void i2c_start(void)
{
    I2C1->CR1 |= (1 << 8);      /* START */
    while (!(I2C1->SR1 & (1 << 0)));  /* wait SB */
}

void i2c_stop(void)
{
    I2C1->CR1 |= (1 << 9);      /* STOP */
}

void i2c_write_addr(uint8_t byte)
{
    /* After SB: write address directly to DR, no TXE wait */
    I2C1->DR = byte;
}

void i2c_write_byte(uint8_t byte)
{
    while (!(I2C1->SR1 & (1 << 7)));  /* wait TXE (EV8) */
    I2C1->DR = byte;
}

uint8_t i2c_read_byte(uint8_t ack)
{
    if (ack)
    {
        I2C1->CR1 |= (1 << 10);  /* ACK */
    }
    else
    {
        I2C1->CR1 &= ~(1 << 10); /* NACK */
    }

    while (!(I2C1->SR1 & (1 << 6)));  /* wait RXNE */
    return I2C1->DR;
}

uint8_t i2c_wait_addr(void)
{
    while (!(I2C1->SR1 & (1 << 1)));  /* wait ADDR */
    return (I2C1->SR1 | I2C1->SR2);   /* read SR1 and SR2 to clear ADDR */
}