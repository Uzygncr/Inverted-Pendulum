#ifndef LCD_H
#define LCD_H

#include "hardware/i2c.h"

void lcd_toggle_enable(uint8_t data);
void lcd_send_byte(uint8_t data, bool is_data);

void lcd_clear(void);
void lcd_set_cursor(int line);
void lcd_print(const char *text);
void lcd_write_line(int line, const char *text);

#endif
