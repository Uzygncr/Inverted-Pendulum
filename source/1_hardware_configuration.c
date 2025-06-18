#include "1_hardware_configuration.h"
#include "2_encoder.h"
#include "2_blink.h"
#include "2_lcd.h"
#include "2_buzzer.h"

#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

#include "pico/stdlib.h"

#include "pico/cyw43_arch.h"

#include <stdio.h>

void hardware_init(void) {
    printf("[debug] Hardware_init...\n");

    // === UART_init ===//
    uart_init(UART_ID, UART_BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // === Buzzer_init ===/
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_enabled(slice_num, true);

    // === Hall_init ===//
    gpio_init(HALL_A_PIN);
    gpio_set_dir(HALL_A_PIN, GPIO_IN);
    gpio_pull_up(HALL_A_PIN);

    gpio_init(HALL_D_PIN);
    gpio_set_dir(HALL_D_PIN, GPIO_IN);
    gpio_pull_up(HALL_D_PIN);

    

    // === Encoder_init ===//
    gpio_init(ENCODER_A_PIN);
    gpio_init(ENCODER_B_PIN);

    gpio_set_dir(ENCODER_A_PIN, GPIO_IN);
    gpio_set_dir(ENCODER_B_PIN, GPIO_IN);

    gpio_set_irq_enabled_with_callback(ENCODER_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_isr);
    gpio_set_irq_enabled_with_callback(ENCODER_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_isr);
    printf("[debug] Hardware_init...Cleared\n");
}


