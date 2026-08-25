#include "spi.h"
#include "stm32f4xx.h"

void spi_init(void)
{
    /* PB13: alternate function, AF5 (SPI2_SCK) */
    GPIOB->MODER  &= ~(3 << 26);
    GPIOB->MODER  |=  (2 << 26);
    GPIOB->AFR[1] |=  (5 << 20);

    /* PB15: alternate function, AF5 (SPI2_MOSI) */
    GPIOB->MODER  &= ~(3U << 30);
    GPIOB->MODER  |=  (2U << 30);
    GPIOB->AFR[1] |=  (5 << 28);

    /* PB12: output (CS, software controlled) */
    GPIOB->MODER  &= ~(3 << 24);
    GPIOB->MODER  |=  (1 << 24);
    GPIOB->BSRR    =  (1 << 12);  /* CS high (inactive) */

    /* Configure SPI2: master mode, CPOL=0, CPHA=0, 8-bit, MSB first */
    /* SSM=1, SSI=1, MSTR=1, BR=001 (fPCLK/4) */
    SPI2->CR1 = (1 << 9) | (1 << 8) | (1 << 2) | (1 << 3);
    SPI2->CR1 |= (1 << 6);  /* SPE: enable SPI */
}

void spi_send(uint8_t byte)
{
    while (!(SPI2->SR & (1 << 1)));  /* wait TXE  */
    SPI2->DR = byte;
    while (!(SPI2->SR & (1 << 1)));  /* wait TXE  */
    while (SPI2->SR & (1 << 7));     /* wait BSY  */
}

uint8_t spi_transfer(uint8_t byte)
{
    while (!(SPI2->SR & (1 << 1)));  /* wait TXE  */
    SPI2->DR = byte;
    while (!(SPI2->SR & (1 << 0)));  /* wait RXNE */
    return SPI2->DR;
}

void spi_cs_low(void)
{
    GPIOB->BSRR = (1 << (12 + 16));  /* PB12 low */
}

void spi_cs_high(void)
{
    GPIOB->BSRR = (1 << 12);         /* PB12 high */
}