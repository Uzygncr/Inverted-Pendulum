#ifndef HARDWARE_H
#define HARDWARE_H

#include "stdbool.h"
//=== Led Gpio ===//
#define LED_PIN CYW43_WL_GPIO_LED_PIN

//=== I2C - LCD ===//
#define I2C_ID        i2c0
#define I2C_SDA_PIN   0
#define I2C_SCL_PIN   1
#define I2C_BAUD_RATE 100000
#define LCD_ADDR      0x27

//Uart ===//
#define UART_ID       uart0
#define UART_TX_PIN   0
#define UART_RX_PIN   1
#define UART_BAUD_RATE 115200

//=== Buzzer ===//
#define BUZZER_PIN     2

// Hall Effect Sensors ===//
#define HALL_A_PIN     3
#define HALL_D_PIN     4

// Stepper Motor (Enable, Direction, Step) ===//
#define MOTOR_EN_PIN   5
#define MOTOR_DIR_PIN  6
#define MOTOR_STEP_PIN 7
// Motor Parametreleri
#define MAX_SPEED 10000
#define MIN_SPEED 500
#define MAX_ACCELERATION 100000
#define MAX_JERK 1000000
#define MIN_ACCEL_DISTANCE_RATIO 0.20
#define MAX_ACCEL_DISTANCE_RATIO 0.30

// Encoder (Quadrature A, B) ===//
#define TICKS_PER_REVOLUTION 1000 // Encoder'ın tam bir turdaki tıklama sayısı
#define RADIUS_MM 350.0 // Çubuğun uzunluğu (mm)
#define DEGREE_PER_TICK (360.0 / TICKS_PER_REVOLUTION)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// Encoder pin tanımları
#define ENCODER_A_PIN  8
#define ENCODER_B_PIN  9

void hardware_init(void);

void lcd_init();

#endif // HARDWARE_H
