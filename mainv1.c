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

    //motor_disable();

    // Core 1’i başlat
    multicore_launch_core1(core1_main);




    

    // PRINT DATA timer
    static repeating_timer_t print_timer;
    add_repeating_timer_ms(100, print_data_callback, NULL, &print_timer);

    // USB okuma için buffer
    char buffer[32];
    int idx = 0;

    while (true) {
        /*if (mode_auto) {
            print_data();
        }*/

        menu_update();
        
        // USB'den komut oku
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

        tight_loop_contents();  // boş döngü, beklemeye devam
    }
}


// --- LQR Kazançları (örnek, MATLAB'dan veya ayarla) ---
#define K_THETA     2.5f   // θ kazancı
#define K_THETA_DOT 0.25f    // θ̇ kazancı
#define K_X         0.01f    // pozisyon kazancı
#define K_X_DOT     0.01f    // hız kazancı

// --- Geçiş sınırları (swing → LQR) ---
#define LQR_THETA_THRESHOLD_DEG   10.0f
#define LQR_OMEGA_THRESHOLD_DPS   30.0f

// --- Swing-up adımları ---
#define NUDGE_STEPS  250
#define WAIT_MS      3

typedef enum {
    STATE_SWING_UP,
    STATE_LQR_BALANCING
} ControlState;

void core1_main() {
    // LED blink timer
    static repeating_timer_t blink_timer;
    add_repeating_timer_ms(250, blink_callback, NULL, &blink_timer);

    // Encoder update timer
    static repeating_timer_t encoder_timer;
    add_repeating_timer_us(-50, update_encoder_callback, NULL, &encoder_timer);

    // Motor döngüsü (örnek rastgele step)


    ControlState state = STATE_SWING_UP;
    int swing_state = 0;

    while (true) {

        calculate_angular_data();
        float theta = encoder_get_angle();       // derece
        float omega = angular_velocity;          // derece/s
        float x = get_position();                // step
        float x_dot = get_speed();               // step/s

        // --- Debug ---
        printf("[state %d] θ=%.2f°, ω=%.2f°/s | x=%d\n", state, theta, omega, (int)x);

        if (state == STATE_SWING_UP) {
            // Swing-up FSM (aynı mantık)
            static enum {WAIT_RIGHT, PUSH_LEFT, WAIT_LEFT, PUSH_RIGHT} swing_phase = WAIT_RIGHT;

            switch (swing_phase) {
                case WAIT_RIGHT:
                    if (theta < -12.0f && omega > 0)
                        swing_phase = PUSH_LEFT;
                    break;
                case PUSH_LEFT:
                    if (fabsf(theta) < 5.0f && omega > 0) {
                        step_motor(NUDGE_STEPS, false);  // sola dürt
                        swing_phase = WAIT_LEFT;
                    }
                    break;
                case WAIT_LEFT:
                    if (theta > 12.0f && omega < 0)
                        swing_phase = PUSH_RIGHT;
                    break;
                case PUSH_RIGHT:
                    if (fabsf(theta) < 5.0f && omega < 0) {
                        step_motor(NUDGE_STEPS, true);  // sağa dürt
                        swing_phase = WAIT_RIGHT;
                    }
                    break;
            }

            // Tepeye ulaşıldıysa LQR'e geç
            /*if (fabsf(theta) < LQR_THETA_THRESHOLD_DEG && fabsf(omega) < LQR_OMEGA_THRESHOLD_DPS) {
                printf("[SWING-UP] Tepe algılandı → LQR'ye geçiliyor!\n");
                state = STATE_LQR_BALANCING;
            }*/
        }

        /*else if (state == STATE_LQR_BALANCING) {
            // --- LQR kontrolü ---
            // u = -K * state
            float u = -(
                K_THETA     * theta +
                K_THETA_DOT * omega +
                K_X         * x +
                K_X_DOT     * x_dot
            );

            int steps = (int)(fabsf(u));
            if (steps > 2000) steps = 2000;  // sınırla
            if (steps > 2) { // gereksiz küçük adımları engelle
                bool direction = (u > 0);  // pozitif u = ileri
                step_motor(steps, direction);
            }
        }*/

        sleep_ms(WAIT_MS);
    }
    


}
