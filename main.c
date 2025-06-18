#include <stdio.h>
#include <math.h>
#include "stdlib.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"

#include "1_hardware_configuration.h"
#include "2_blink.h"
#include "2_encoder.h"
#include "2_motor.h"
#include "2_buzzer.h"
#include "2_lcd.h"
#include "3_menu.h"
#include "3_talker.h"
#include "3_usb.h"
#include "4_swing_up.h"
#include "4_lqr.h"
#include "5_controller.h"

// === LQR KAZANÇLARI (şimdilik kullanılmıyor ama saklı)
#define K_THETA        2.5f
#define K_THETA_DOT    0.25f
#define K_THETA_DDOT   0.01f
#define K_X            0.025f
#define K_X_DOT        0.025f

#define LQR_THETA_THRESHOLD_DEG   40.0f
#define LQR_OMEGA_THRESHOLD_DPS   30.0f
#define LQR_STABLE_TIME_US        1

#define NUDGE_STEPS  500
#define WAIT_MS      3

typedef enum {
    STATE_SWING_UP,
    STATE_LQR_BALANCING  // Şu an kullanılmıyor
} ControlState;

typedef struct {
    float theta;
    float omega;
    float x;
    int swing_phase;
} monitor_data_t;

volatile monitor_data_t monitor_data;

void core1_main();

// === Yazdırma sadece Core 0 tarafından yapılır ===
bool encoder_print_data_callback(repeating_timer_t *rt) {
    float theta = monitor_data.theta;
    float omega = monitor_data.omega;
    float x     = monitor_data.x;
    int phase   = monitor_data.swing_phase;

    /*printf("[Swing-Up] θ=%.2f°, ω=%.2f°/s | x=%.0f | phase=%d\n",
           theta, omega, x, phase);*/

    return true;
}

int main() {
    stdio_init_all();
    sleep_ms(1000);
    printf("[debug] Stdio_init...Cleared\n");
    hardware_init();

    blink_init();
    lcd_init();
    menu_init();
    motor_init();
    buzzer_play_mario_theme();

    multicore_launch_core1(core1_main);

    static repeating_timer_t print_timer;
    add_repeating_timer_ms(100, encoder_print_data_callback, NULL, &print_timer);

    char buffer[32];
    int idx = 0;

    while (true) {
        menu_update();
        int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT) {
            if (ch == '\n' || ch == '\r') {
                buffer[idx] = '\0';
                if (idx > 0) {
                    menu_handle_command(buffer);
                    idx = 0;
                }
            } else if (idx < sizeof(buffer) - 1) {
                buffer[idx++] = (char)ch;
            }
        }
        tight_loop_contents();
    }
}


void core1_main() {
    static repeating_timer_t blink_timer;
    add_repeating_timer_ms(250, blink_callback, NULL, &blink_timer);

    static repeating_timer_t encoder_timer;
    add_repeating_timer_us(-50, update_encoder_callback, NULL, &encoder_timer);

    while (true) {
        controller_run();  // swing-up ↔ LQR geçişini burası kontrol eder
    }
}


