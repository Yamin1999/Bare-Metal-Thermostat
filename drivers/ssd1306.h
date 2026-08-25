#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

/* Display dimensions */
#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64

/* Colors */
#define SSD1306_WHITE   1
#define SSD1306_BLACK   0

/* Initialization / control */
void ssd1306_init(void);
void ssd1306_update(void);
void ssd1306_clear(void);

/* Drawing primitives */
void ssd1306_draw_pixel(uint8_t x, uint8_t y, uint8_t color);
void ssd1306_draw_char(uint8_t x, uint8_t y, char ch, uint8_t color);
void ssd1306_draw_string(uint8_t x, uint8_t y, const char *str, uint8_t color);
void ssd1306_draw_hline(uint8_t x, uint8_t y, uint8_t w, uint8_t color);
void ssd1306_draw_vline(uint8_t x, uint8_t y, uint8_t h, uint8_t color);
void ssd1306_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void ssd1306_draw_filled_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

#endif