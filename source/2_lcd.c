#include "1_hardware_configuration.h"
#include "2_lcd.h"

#include "pico/stdlib.h"

#include <string.h>
#include <stdio.h>

void lcd_init(){
    printf("[debug] lcd_init...\n");
    i2c_init(I2C_ID, I2C_BAUD_RATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    sleep_ms(50);
    lcd_send_byte(0x33, false);
    lcd_send_byte(0x32, false);
    lcd_send_byte(0x28, false);
    lcd_send_byte(0x0C, false);
    lcd_send_byte(0x06, false);
    lcd_send_byte(0x01, false);
    sleep_ms(5);
    printf("[debug] lcd_initialized...Cleared\n");
}


// Enable pini tetikleme
void lcd_toggle_enable(uint8_t data) {
    sleep_us(600);
    i2c_write_blocking(I2C_ID, LCD_ADDR, &data, 1, false);
    data |= 0x04;
    i2c_write_blocking(I2C_ID, LCD_ADDR, &data, 1, false);
    data &= ~0x04;
    i2c_write_blocking(I2C_ID, LCD_ADDR, &data, 1, false);
    sleep_us(600);
}

// Komut ya da veri gönderme
void lcd_send_byte(uint8_t data, bool is_data) {
    uint8_t mode = is_data ? 0x01 : 0x00;
    uint8_t high = (data & 0xF0) | mode | 0x08;
    uint8_t low  = ((data << 4) & 0xF0) | mode | 0x08;

    i2c_write_blocking(I2C_ID, LCD_ADDR, &high, 1, false);
    lcd_toggle_enable(high);
    i2c_write_blocking(I2C_ID, LCD_ADDR, &low, 1, false);
    lcd_toggle_enable(low);
}



// Temizleme
void lcd_clear(void) {
    lcd_send_byte(0x01, false);
    sleep_ms(2);
}

// İmleç konumu
void lcd_set_cursor(int line) {
    lcd_send_byte((line == 0) ? 0x80 : 0xC0, false);
}

// Yazı yazma
void lcd_print(const char *text) {
    while (*text) {
        lcd_send_byte(*text++, true);
    }
}

void lcd_write_line(int line, const char *text) {
    lcd_set_cursor(line);
    lcd_print(text);
}
