#include "bme280.h"
#include "i2c.h"
#include "spi.h"
#include "systick.h"
#include "stm32f4xx.h"

/* Calibration coefficients */
static uint16_t dig_T1;
static int16_t  dig_T2;
static int16_t  dig_T3;

static uint16_t dig_P1;
static int16_t  dig_P2;
static int16_t  dig_P3;
static int16_t  dig_P4;
static int16_t  dig_P5;
static int16_t  dig_P6;
static int16_t  dig_P7;
static int16_t  dig_P8;
static int16_t  dig_P9;

static uint8_t  dig_H1;
static int16_t  dig_H2;
static uint8_t  dig_H3;
static int16_t  dig_H4;
static int16_t  dig_H5;
static int8_t   dig_H6;

static int32_t t_fine;

void bme280_init_i2c(void) {
    uint8_t msb, lsb;

    i2c_init();

    /* Read temperature calibration coefficients 0x88-0x8D */
    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 0);
    i2c_wait_addr();
    i2c_write_byte(0x88);
    while (!(I2C1->SR1 & (1 << 2)));

    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 1);
    i2c_wait_addr();

    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_T1 = (msb << 8) | lsb;
    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_T2 = (msb << 8) | lsb;
    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_T3 = (msb << 8) | lsb;

    /* Pressure calibration coefficients 0x8E-0x9F (continues from burst) */
    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_P1 = (msb << 8) | lsb;
    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_P2 = (msb << 8) | lsb;
    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_P3 = (msb << 8) | lsb;
    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_P4 = (msb << 8) | lsb;
    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_P5 = (msb << 8) | lsb;
    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_P6 = (msb << 8) | lsb;
    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_P7 = (msb << 8) | lsb;
    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_P8 = (msb << 8) | lsb;
    lsb = i2c_read_byte(1); msb = i2c_read_byte(0);
    dig_P9 = (msb << 8) | lsb;
    i2c_stop();

    /* Read humidity calibration: dig_H1 from 0xA1 */
    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 0);
    i2c_wait_addr();
    i2c_write_byte(0xA1);
    while (!(I2C1->SR1 & (1 << 2)));

    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 1);
    i2c_wait_addr();
    dig_H1 = i2c_read_byte(0);
    i2c_stop();

    /* Read humidity calibration: dig_H2..H6 from 0xE1-0xE7 */
    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 0);
    i2c_wait_addr();
    i2c_write_byte(0xE1);
    while (!(I2C1->SR1 & (1 << 2)));

    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 1);
    i2c_wait_addr();

    lsb = i2c_read_byte(1); msb = i2c_read_byte(1);
    dig_H2 = (msb << 8) | lsb;
    dig_H3 = i2c_read_byte(1);
    msb = i2c_read_byte(1);
    lsb = i2c_read_byte(1);
    dig_H4 = (msb << 4) | (lsb & 0x0F);
    msb = i2c_read_byte(1);
    dig_H5 = (msb << 4) | (lsb >> 4);
    dig_H6 = (int8_t)i2c_read_byte(0);
    i2c_stop();

    /* Configure humidity oversampling (must be written before ctrl_meas) */
    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 0);
    i2c_wait_addr();
    i2c_write_byte(0xF2);  /* ctrl_hum register */
    while (!(I2C1->SR1 & (1 << 7)));
    i2c_write_byte(0x01);  /* osrs_h = 001 (×1) */
    while (!(I2C1->SR1 & (1 << 2)));
    i2c_stop();

    /* Configure sensor: forced mode, temp ×1, pressure ×1 */
    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 0);
    i2c_wait_addr();
    i2c_write_byte(0xF4);  /* ctrl_meas register */
    while (!(I2C1->SR1 & (1 << 7)));
    i2c_write_byte(0x25);  /* osrs_t=001, osrs_p=001, mode=01 (forced) */
    while (!(I2C1->SR1 & (1 << 2)));
    i2c_stop();

}

void bme280_wait_ready_i2c(void) {
    uint8_t status;

    do {
        i2c_start();
        i2c_write_addr((BME280_ADDR << 1) | 0);
        i2c_wait_addr();
        i2c_write_byte(0xF3);  /* status register */
        while (!(I2C1->SR1 & (1 << 2)));

        i2c_start();
        i2c_write_addr((BME280_ADDR << 1) | 1);
        i2c_wait_addr();

        status = i2c_read_byte(0);
        i2c_stop();
    } while (status & (1 << 3));  /* wait until measuring bit is clear */
}

int32_t bme280_read_temp_i2c(void) {
    uint8_t msb, lsb, xlsb;
    int32_t adc_T, var1, var2, T;

    /* Trigger measurement in forced mode (all three: temp+press+hum) */
    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 0);
    i2c_wait_addr();
    i2c_write_byte(0xF4);
    while (!(I2C1->SR1 & (1 << 7)));
    i2c_write_byte(0x25);  /* osrs_t=001, osrs_p=001, mode=01 */
    while (!(I2C1->SR1 & (1 << 2)));
    i2c_stop();

    /* Wait for measurement to complete */
    bme280_wait_ready_i2c();

    /* Read temperature registers 0xFA-0xFC */
    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 0);
    i2c_wait_addr();
    i2c_write_byte(0xFA);
    while (!(I2C1->SR1 & (1 << 2)));

    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 1);
    i2c_wait_addr();

    msb  = i2c_read_byte(1);
    lsb  = i2c_read_byte(1);
    xlsb = i2c_read_byte(0);
    i2c_stop();

    /* Combine into 20-bit value */
    adc_T = ((int32_t)msb << 12) | ((int32_t)lsb << 4) | ((int32_t)xlsb >> 4);

    /* Apply compensation formula from BME280 datasheet */
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    return T;  /* Temperature in 0.01°C (e.g., 2534 = 25.34°C) */
}

uint32_t bme280_read_pressure_i2c(void) {
    uint8_t msb, lsb, xlsb;
    int32_t adc_P;
    int64_t var1, var2, p;

    /* Read pressure registers 0xF7-0xF9 */
    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 0);
    i2c_wait_addr();
    i2c_write_byte(0xF7);
    while (!(I2C1->SR1 & (1 << 2)));

    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 1);
    i2c_wait_addr();

    msb  = i2c_read_byte(1);
    lsb  = i2c_read_byte(1);
    xlsb = i2c_read_byte(0);
    i2c_stop();

    adc_P = ((int32_t)msb << 12) | ((int32_t)lsb << 4) | ((int32_t)xlsb >> 4);

    /* Compensation formula (requires t_fine from temperature reading) */
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) return 0;

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    return (uint32_t)p;  /* Pressure in Pa/256 (e.g., 25600000 = 100000 Pa) */
}

uint32_t bme280_read_humidity_i2c(void) {
    uint8_t msb, lsb;
    int32_t adc_H, v_x1_u32r;

    /* Read humidity registers 0xFD-0xFE */
    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 0);
    i2c_wait_addr();
    i2c_write_byte(0xFD);
    while (!(I2C1->SR1 & (1 << 2)));

    i2c_start();
    i2c_write_addr((BME280_ADDR << 1) | 1);
    i2c_wait_addr();

    msb = i2c_read_byte(1);
    lsb = i2c_read_byte(0);
    i2c_stop();

    adc_H = (msb << 8) | lsb;

    /* Compensation formula (requires t_fine from temperature reading) */
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) +
                   ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) *
                   (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                   ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                               ((int32_t)dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return (uint32_t)(v_x1_u32r >> 12);  /* Humidity in %RH/1024 (e.g., 51200 = 50%) */
}

void bme280_init_spi(void) {
    uint8_t msb, lsb;

    spi_init();

    /* Small delay for sensor power-up */
    delay_ms(100);

    /* Read temperature calibration coefficients from 0x88-0x8D */
    spi_cs_low();
    spi_transfer(0x88 | 0x80);
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_T1 = (msb << 8) | lsb;
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_T2 = (msb << 8) | lsb;
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_T3 = (msb << 8) | lsb;
    spi_cs_high();

    /* Read pressure calibration coefficients from 0x8E-0x9F */
    spi_cs_low();
    spi_transfer(0x8E | 0x80);
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_P1 = (msb << 8) | lsb;
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_P2 = (msb << 8) | lsb;
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_P3 = (msb << 8) | lsb;
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_P4 = (msb << 8) | lsb;
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_P5 = (msb << 8) | lsb;
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_P6 = (msb << 8) | lsb;
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_P7 = (msb << 8) | lsb;
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_P8 = (msb << 8) | lsb;
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_P9 = (msb << 8) | lsb;
    spi_cs_high();

    /* Read humidity calibration coefficients */
    spi_cs_low();
    spi_transfer(0xA1 | 0x80);
    dig_H1 = spi_transfer(0x00);
    spi_cs_high();

    spi_cs_low();
    spi_transfer(0xE1 | 0x80);
    lsb = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    dig_H2 = (msb << 8) | lsb;
    dig_H3 = spi_transfer(0x00);
    msb = spi_transfer(0x00);
    lsb = spi_transfer(0x00);
    dig_H4 = (msb << 4) | (lsb & 0x0F);
    msb = spi_transfer(0x00);
    dig_H5 = (msb << 4) | (lsb >> 4);
    dig_H6 = (int8_t)spi_transfer(0x00);
    spi_cs_high();

    /* Configure humidity oversampling */
    spi_cs_low();
    spi_transfer(0xF2 & 0x7F);
    spi_transfer(0x01);  /* osrs_h = 001 (×1) */
    spi_cs_high();

    /* Configure sensor: forced mode, temp and pressure oversampling ×1 */
    spi_cs_low();
    spi_transfer(0xF4 & 0x7F);
    spi_transfer(0x25);  /* osrs_t = 001, osrs_p = 001, mode = 01 (forced) */
    spi_cs_high();
}

int32_t bme280_read_temp_spi(void) {
    uint8_t msb, lsb, xlsb, status;
    int32_t adc_T, var1, var2, T;

    /* Trigger measurement in forced mode */
    spi_cs_low();
    spi_transfer(0xF4 & 0x7F);
    spi_transfer(0x25);
    spi_cs_high();

    /* Wait for measurement to complete */
    do {
        spi_cs_low();
        spi_transfer(0xF3 | 0x80);  /* Read status register */
        status = spi_transfer(0x00);
        spi_cs_high();
    } while (status & (1 << 3));

    /* Read temperature registers 0xFA-0xFC */
    spi_cs_low();
    spi_transfer(0xFA | 0x80);
    msb  = spi_transfer(0x00);
    lsb  = spi_transfer(0x00);
    xlsb = spi_transfer(0x00);
    spi_cs_high();

    /* Combine into 20-bit value */
    adc_T = ((int32_t)msb << 12) | ((int32_t)lsb << 4) | ((int32_t)xlsb >> 4);

    /* Apply compensation formula from BME280 datasheet */
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    return T;  /* Temperature in 0.01°C resolution (e.g., 2534 = 25.34°C) */
}

uint32_t bme280_read_pressure_spi(void) {
    uint8_t msb, lsb, xlsb;
    int32_t adc_P;
    int64_t var1, var2, p;

    /* Read pressure registers 0xF7-0xF9 */
    spi_cs_low();
    spi_transfer(0xF7 | 0x80);
    msb  = spi_transfer(0x00);
    lsb  = spi_transfer(0x00);
    xlsb = spi_transfer(0x00);
    spi_cs_high();

    /* Combine into 20-bit value */
    adc_P = ((int32_t)msb << 12) | ((int32_t)lsb << 4) | ((int32_t)xlsb >> 4);

    /* Apply compensation formula (requires t_fine from temperature reading) */
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0)
        return 0;

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    return (uint32_t)p;  /* Pressure in Pa/256 (e.g., 25600000 = 100000 Pa = 1000 hPa) */
}

uint32_t bme280_read_humidity_spi(void) {
    uint8_t msb, lsb;
    int32_t adc_H, v_x1_u32r;

    /* Read humidity registers 0xFD-0xFE */
    spi_cs_low();
    spi_transfer(0xFD | 0x80);
    msb = spi_transfer(0x00);
    lsb = spi_transfer(0x00);
    spi_cs_high();

    /* Combine into 16-bit value */
    adc_H = (msb << 8) | lsb;

    /* Apply compensation formula (requires t_fine from temperature reading) */
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) +
                   ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) *
                   (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                   ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                               ((int32_t)dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return (uint32_t)(v_x1_u32r >> 12);  /* Humidity in %RH/1024 (e.g., 51200 = 50% RH) */
}