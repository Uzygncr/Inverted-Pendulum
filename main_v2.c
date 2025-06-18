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

int counter_Core0 = 0;
int counter_Core1 = 1;
void core1_main();

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
    add_repeating_timer_ms(100, print_data_callback, NULL, &print_timer);

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

#define K_THETA     2.5f
#define K_THETA_DOT 0.25f
#define K_X         0.01f
#define K_X_DOT     0.01f

#define LQR_THETA_THRESHOLD_DEG   10.0f
#define LQR_OMEGA_THRESHOLD_DPS   30.0f

#define NUDGE_STEPS  250
#define WAIT_MS      3

typedef enum {
    STATE_SWING_UP,
    STATE_LQR_BALANCING
} ControlState;

void core1_main() {
    static repeating_timer_t blink_timer;
    add_repeating_timer_ms(250, blink_callback, NULL, &blink_timer);

    static repeating_timer_t encoder_timer;
    add_repeating_timer_us(-50, update_encoder_callback, NULL, &encoder_timer);

    ControlState state = STATE_SWING_UP;
    absolute_time_t lqr_entry_timer = nil_time;

    while (true) {
        calculate_angular_data();
        float theta = encoder_get_angle();
        float omega = angular_velocity;
        float x = get_position();
        float x_dot = get_speed();

        float theta_rad = theta * (M_PI / 180.0f);
        float omega_rad = omega * (M_PI / 180.0f);
        float m = 0.12f, l = 0.35f, g = 9.81f;
        float Ep = m * g * l * (1.0f - cosf(theta_rad));
        float Ek = 0.5f * m * l * l * omega_rad * omega_rad;
        float E_total = Ep + Ek;
        float E_desired = m * g * l;
        float energy_ratio = E_total / E_desired;
        if (energy_ratio > 1.2f) energy_ratio = 1.2f;

        float theta_peak_limit = 12.0f + 8.0f * (energy_ratio - 0.5f);
        float theta_center_range = 5.0f + 4.0f * (1.0f - energy_ratio);
        if (theta_center_range < 2.0f) theta_center_range = 2.0f;

        printf("[state %d] θ=%.2f°, ω=%.2f°/s | x=%d\n", state, theta, omega, (int)x);

        /*if (state == STATE_SWING_UP) {
            static enum {WAIT_RIGHT, PUSH_LEFT, WAIT_LEFT, PUSH_RIGHT} swing_phase = WAIT_RIGHT;
            float boost = fabsf(theta) / 15.0f;
            int step_amount = (int)(NUDGE_STEPS * (1.0f + boost));

            switch (swing_phase) {
                case WAIT_RIGHT:
                    if (theta < -theta_peak_limit && omega > 0)
                        swing_phase = PUSH_LEFT;
                    break;
                case PUSH_LEFT:
                    if (fabsf(theta) < theta_center_range && omega > 0) {
                        step_motor(step_amount, false);
                        swing_phase = WAIT_LEFT;
                    }
                    break;
                case WAIT_LEFT:
                    if (theta > theta_peak_limit && omega < 0)
                        swing_phase = PUSH_RIGHT;
                    break;
                case PUSH_RIGHT:
                    if (fabsf(theta) < theta_center_range && omega < 0) {
                        step_motor(step_amount, true);
                        swing_phase = WAIT_RIGHT;
                    }
                    break;
            }*/

            // === Tepe kontrolü ===
            float theta_from_top = fabsf(fmodf(theta - 180.0f + 360.0f, 360.0f) - 180.0f);
            if (theta_from_top < LQR_THETA_THRESHOLD_DEG && fabsf(omega) < LQR_OMEGA_THRESHOLD_DPS) {
                if (is_nil_time(lqr_entry_timer)) {
                    lqr_entry_timer = get_absolute_time();
                } else {
                    int64_t elapsed_us = absolute_time_diff_us(lqr_entry_timer, get_absolute_time());
                    if (elapsed_us > 500 * 1000) {
                        printf("[SWING-UP] Tepe stabil → LQR'ye geçiliyor\n");
                        state = STATE_LQR_BALANCING;
                    }
                }
            } else {
                lqr_entry_timer = nil_time;
            }

        /*} else if (state == STATE_LQR_BALANCING) {
            float u = -(
                K_THETA     * theta +
                K_THETA_DOT * omega +
                K_X         * x +
                K_X_DOT     * x_dot
            );

            int steps = (int)(fabsf(u));
            if (steps > 2000) steps = 2000;
            if (steps > 2) {
                bool direction = (u < 0);  // u > 0 ise sola dürt (motor yönüne göre)
                step_motor(steps, direction);
            }
        }*/

        sleep_ms(WAIT_MS);
    }
}

