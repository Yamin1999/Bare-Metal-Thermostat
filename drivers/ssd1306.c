#include "ssd1306.h"
#include "spi.h"
#include "systick.h"
#include "stm32f4xx.h"

/* ── Framebuffer ─────────────────────────────────────────────── */
/* 128 x 64 pixels, 1 bit per pixel = 1024 bytes                */
/* Organised in 8 pages of 128 columns (page = 8 rows)          */
static uint8_t framebuffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/* ── 5×7 ASCII Font (chars 32-126) ───────────────────────────── */
static const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* ' ' (32) */
    {0x00,0x00,0x5F,0x00,0x00}, /* '!'      */
    {0x00,0x07,0x00,0x07,0x00}, /* '"'      */
    {0x14,0x7F,0x14,0x7F,0x14}, /* '#'      */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* '$'      */
    {0x23,0x13,0x08,0x64,0x62}, /* '%'      */
    {0x36,0x49,0x55,0x22,0x50}, /* '&'      */
    {0x00,0x05,0x03,0x00,0x00}, /* '''      */
    {0x00,0x1C,0x22,0x41,0x00}, /* '('      */
    {0x00,0x41,0x22,0x1C,0x00}, /* ')'      */
    {0x14,0x08,0x3E,0x08,0x14}, /* '*'      */
    {0x08,0x08,0x3E,0x08,0x08}, /* '+'      */
    {0x00,0x50,0x30,0x00,0x00}, /* ','      */
    {0x08,0x08,0x08,0x08,0x08}, /* '-'      */
    {0x00,0x60,0x60,0x00,0x00}, /* '.'      */
    {0x20,0x10,0x08,0x04,0x02}, /* '/'      */
    {0x3E,0x51,0x49,0x45,0x3E}, /* '0'      */
    {0x00,0x42,0x7F,0x40,0x00}, /* '1'      */
    {0x42,0x61,0x51,0x49,0x46}, /* '2'      */
    {0x21,0x41,0x45,0x4B,0x31}, /* '3'      */
    {0x18,0x14,0x12,0x7F,0x10}, /* '4'      */
    {0x27,0x45,0x45,0x45,0x39}, /* '5'      */
    {0x3C,0x4A,0x49,0x49,0x30}, /* '6'      */
    {0x01,0x71,0x09,0x05,0x03}, /* '7'      */
    {0x36,0x49,0x49,0x49,0x36}, /* '8'      */
    {0x06,0x49,0x49,0x29,0x1E}, /* '9'      */
    {0x00,0x36,0x36,0x00,0x00}, /* ':'      */
    {0x00,0x56,0x36,0x00,0x00}, /* ';'      */
    {0x08,0x14,0x22,0x41,0x00}, /* '<'      */
    {0x14,0x14,0x14,0x14,0x14}, /* '='      */
    {0x00,0x41,0x22,0x14,0x08}, /* '>'      */
    {0x02,0x01,0x51,0x09,0x06}, /* '?'      */
    {0x32,0x49,0x79,0x41,0x3E}, /* '@'      */
    {0x7E,0x11,0x11,0x11,0x7E}, /* 'A' (65) */
    {0x7F,0x49,0x49,0x49,0x36}, /* 'B'      */
    {0x3E,0x41,0x41,0x41,0x22}, /* 'C'      */
    {0x7F,0x41,0x41,0x22,0x1C}, /* 'D'      */
    {0x7F,0x49,0x49,0x49,0x41}, /* 'E'      */
    {0x7F,0x09,0x09,0x09,0x01}, /* 'F'      */
    {0x3E,0x41,0x49,0x49,0x7A}, /* 'G'      */
    {0x7F,0x08,0x08,0x08,0x7F}, /* 'H'      */
    {0x00,0x41,0x7F,0x41,0x00}, /* 'I'      */
    {0x20,0x40,0x41,0x3F,0x01}, /* 'J'      */
    {0x7F,0x08,0x14,0x22,0x41}, /* 'K'      */
    {0x7F,0x40,0x40,0x40,0x40}, /* 'L'      */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* 'M'      */
    {0x7F,0x04,0x08,0x10,0x7F}, /* 'N'      */
    {0x3E,0x41,0x41,0x41,0x3E}, /* 'O'      */
    {0x7F,0x09,0x09,0x09,0x06}, /* 'P'      */
    {0x3E,0x41,0x51,0x21,0x5E}, /* 'Q'      */
    {0x7F,0x09,0x19,0x29,0x46}, /* 'R'      */
    {0x46,0x49,0x49,0x49,0x31}, /* 'S'      */
    {0x01,0x01,0x7F,0x01,0x01}, /* 'T'      */
    {0x3F,0x40,0x40,0x40,0x3F}, /* 'U'      */
    {0x1F,0x20,0x40,0x20,0x1F}, /* 'V'      */
    {0x3F,0x40,0x38,0x40,0x3F}, /* 'W'      */
    {0x63,0x14,0x08,0x14,0x63}, /* 'X'      */
    {0x07,0x08,0x70,0x08,0x07}, /* 'Y'      */
    {0x61,0x51,0x49,0x45,0x43}, /* 'Z'      */
    {0x00,0x7F,0x41,0x41,0x00}, /* '['      */
    {0x02,0x04,0x08,0x10,0x20}, /* '\'      */
    {0x00,0x41,0x41,0x7F,0x00}, /* ']'      */
    {0x04,0x02,0x01,0x02,0x04}, /* '^'      */
    {0x40,0x40,0x40,0x40,0x40}, /* '_'      */
    {0x00,0x01,0x02,0x04,0x00}, /* '`'      */
    {0x20,0x54,0x54,0x54,0x78}, /* 'a' (97) */
    {0x7F,0x48,0x44,0x44,0x38}, /* 'b'      */
    {0x38,0x44,0x44,0x44,0x20}, /* 'c'      */
    {0x38,0x44,0x44,0x48,0x7F}, /* 'd'      */
    {0x38,0x54,0x54,0x54,0x18}, /* 'e'      */
    {0x08,0x7E,0x09,0x01,0x02}, /* 'f'      */
    {0x0C,0x52,0x52,0x52,0x3E}, /* 'g'      */
    {0x7F,0x08,0x04,0x04,0x78}, /* 'h'      */
    {0x00,0x44,0x7D,0x40,0x00}, /* 'i'      */
    {0x20,0x40,0x44,0x3D,0x00}, /* 'j'      */
    {0x7F,0x10,0x28,0x44,0x00}, /* 'k'      */
    {0x00,0x41,0x7F,0x40,0x00}, /* 'l'      */
    {0x7C,0x04,0x18,0x04,0x78}, /* 'm'      */
    {0x7C,0x08,0x04,0x04,0x78}, /* 'n'      */
    {0x38,0x44,0x44,0x44,0x38}, /* 'o'      */
    {0x7C,0x14,0x14,0x14,0x08}, /* 'p'      */
    {0x08,0x14,0x14,0x18,0x7C}, /* 'q'      */
    {0x7C,0x08,0x04,0x04,0x08}, /* 'r'      */
    {0x48,0x54,0x54,0x54,0x20}, /* 's'      */
    {0x04,0x3F,0x44,0x40,0x20}, /* 't'      */
    {0x3C,0x40,0x40,0x20,0x7C}, /* 'u'      */
    {0x1C,0x20,0x40,0x20,0x1C}, /* 'v'      */
    {0x3C,0x40,0x30,0x40,0x3C}, /* 'w'      */
    {0x44,0x28,0x10,0x28,0x44}, /* 'x'      */
    {0x0C,0x50,0x50,0x50,0x3C}, /* 'y'      */
    {0x44,0x64,0x54,0x4C,0x44}, /* 'z'      */
    {0x00,0x08,0x36,0x41,0x00}, /* '{'      */
    {0x00,0x00,0x7F,0x00,0x00}, /* '|'      */
    {0x00,0x41,0x36,0x08,0x00}, /* '}'      */
    {0x08,0x04,0x08,0x10,0x08}, /* '~'      */
};

/* ── Private: GPIO helpers (hardcoded pins) ──────────────────── */
/* DC = PB14, RST = PB2 */

static void dc_low(void)  { GPIOB->BSRR = (1 << (14 + 16)); }
static void dc_high(void) { GPIOB->BSRR = (1 << 14); }
static void rst_low(void)  { GPIOB->BSRR = (1 << (2 + 16)); }
static void rst_high(void) { GPIOB->BSRR = (1 << 2); }

/* ── Private: SSD1306 command helpers ────────────────────────── */

static void ssd1306_write_cmd(uint8_t cmd) {
    dc_low();              /* DC low = command */
    spi_cs_low();
    spi_send(cmd);
    spi_cs_high();
}

static void ssd1306_write_data(const uint8_t *data, uint16_t len) {
    dc_high();             /* DC high = data */
    spi_cs_low();
    for (uint16_t i = 0; i < len; i++) {
        spi_send(data[i]);
    }
    spi_cs_high();
}


/* ── Public API ──────────────────────────────────────────────── */

void ssd1306_init(void) {
    /* PB14 (DC) and PB2 (RST) as GPIO output */
    GPIOB->MODER &= ~(3U << (14 * 2));
    GPIOB->MODER |=  (1U << (14 * 2));
    GPIOB->MODER &= ~(3U << (2 * 2));
    GPIOB->MODER |=  (1U << (2 * 2));

    /* Hardware reset */
    rst_high();
    delay_ms(2);
    rst_low();
    delay_ms(2);
    rst_high();
    delay_ms(2);

    /* Initialization sequence (datasheet recommended) */
    ssd1306_write_cmd(0xAE);  /* Display OFF                      */
    ssd1306_write_cmd(0xD5);  /* Set display clock divide          */
    ssd1306_write_cmd(0x80);  /* Suggested ratio 0x80              */
    ssd1306_write_cmd(0xA8);  /* Set multiplex                     */
    ssd1306_write_cmd(0x3F);  /* 64 lines (height - 1)             */
    ssd1306_write_cmd(0xD3);  /* Set display offset                */
    ssd1306_write_cmd(0x00);  /* No offset                         */
    ssd1306_write_cmd(0x40);  /* Set start line = 0                */
    ssd1306_write_cmd(0x8D);  /* Charge pump                       */
    ssd1306_write_cmd(0x14);  /* Enable charge pump                */
    ssd1306_write_cmd(0x20);  /* Memory addressing mode            */
    ssd1306_write_cmd(0x00);  /* Horizontal addressing mode        */
    ssd1306_write_cmd(0xA1);  /* Segment re-map: col 127 = SEG0    */
    ssd1306_write_cmd(0xC8);  /* COM output scan: remapped         */
    ssd1306_write_cmd(0xDA);  /* Set COM pins hardware config      */
    ssd1306_write_cmd(0x12);  /* Alternative COM pin config        */
    ssd1306_write_cmd(0x81);  /* Set contrast                      */
    ssd1306_write_cmd(0xCF);  /* Max contrast                      */
    ssd1306_write_cmd(0xD9);  /* Set pre-charge period             */
    ssd1306_write_cmd(0xF1);  /* Phase1=15, Phase2=1               */
    ssd1306_write_cmd(0xDB);  /* Set VCOMH deselect level          */
    ssd1306_write_cmd(0x40);  /* ~0.77 x VCC                       */
    ssd1306_write_cmd(0xA4);  /* Entire display ON (follow RAM)    */
    ssd1306_write_cmd(0xA6);  /* Normal display (not inverted)     */
    ssd1306_write_cmd(0xAF);  /* Display ON                        */

    /* Clear screen */
    ssd1306_clear();
    ssd1306_update();
}

void ssd1306_update(void) {
    /* Set column address range: 0 → 127 */
    ssd1306_write_cmd(0x21);
    ssd1306_write_cmd(0x00);
    ssd1306_write_cmd(0x7F);

    /* Set page address range: 0 → 7 */
    ssd1306_write_cmd(0x22);
    ssd1306_write_cmd(0x00);
    ssd1306_write_cmd(0x07);

    /* Write entire framebuffer */
    ssd1306_write_data(framebuffer, sizeof(framebuffer));
}

void ssd1306_clear(void) {
    for (uint16_t i = 0; i < sizeof(framebuffer); i++) {
        framebuffer[i] = 0x00;
    }
}

void ssd1306_draw_pixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    /* Each byte in framebuffer represents 8 vertical pixels (1 page) */
    /* framebuffer layout: page 0 cols 0-127, page 1 cols 0-127, ... */
    if (color) {
        framebuffer[x + (y / 8) * SSD1306_WIDTH] |=  (1 << (y & 7));
    } else {
        framebuffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y & 7));
    }
}

void ssd1306_draw_char(uint8_t x, uint8_t y, char ch, uint8_t color) {
    if (ch < 32 || ch > 126) ch = '?';

    const uint8_t *glyph = font5x7[ch - 32];

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (uint8_t row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                ssd1306_draw_pixel(x + col, y + row, color);
            } else {
                ssd1306_draw_pixel(x + col, y + row, !color);
            }
        }
    }
    /* 1-pixel spacing between characters */
    for (uint8_t row = 0; row < 7; row++) {
        ssd1306_draw_pixel(x + 5, y + row, !color);
    }
}

void ssd1306_draw_string(uint8_t x, uint8_t y, const char *str, uint8_t color) {
    while (*str) {
        if (x + 6 > SSD1306_WIDTH) {
            /* Wrap to next line */
            x = 0;
            y += 8;
        }
        if (y + 7 > SSD1306_HEIGHT) break;  /* No more vertical space */

        ssd1306_draw_char(x, y, *str, color);
        x += 6;  /* 5 pixels + 1 spacing */
        str++;
    }
}

void ssd1306_draw_hline(uint8_t x, uint8_t y, uint8_t w, uint8_t color) {
    for (uint8_t i = 0; i < w; i++) {
        ssd1306_draw_pixel(x + i, y, color);
    }
}

void ssd1306_draw_vline(uint8_t x, uint8_t y, uint8_t h, uint8_t color) {
    for (uint8_t i = 0; i < h; i++) {
        ssd1306_draw_pixel(x, y + i, color);
    }
}

void ssd1306_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    ssd1306_draw_hline(x, y, w, color);
    ssd1306_draw_hline(x, y + h - 1, w, color);
    ssd1306_draw_vline(x, y, h, color);
    ssd1306_draw_vline(x + w - 1, y, h, color);
}

void ssd1306_draw_filled_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    for (uint8_t i = 0; i < h; i++) {
        ssd1306_draw_hline(x, y + i, w, color);
    }
}
