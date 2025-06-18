#include "4_swing_up.h"
#include "2_encoder.h"
#include "2_motor.h"
#include "math.h"
#include "pico/stdlib.h"
#include "1_hardware_configuration.h"  // Eğer pin tanımları buradaysa

#define NUDGE_STEPS  250
#define WAIT_MS      3

typedef enum {WAIT_RIGHT, PUSH_LEFT, WAIT_LEFT, PUSH_RIGHT} SwingPhase;
static SwingPhase swing_phase = WAIT_RIGHT;

static bool initial_kick_done = false;

extern volatile struct {
    float theta;
    float omega;
    float x;
    int swing_phase;
} monitor_data;

void swing_up_control() {
    calculate_angular_data();

    float theta = encoder_get_angle();
    float omega = angular_velocity;
    float x = get_position();

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

    float boost = fabsf(theta) / 15.0f;
    int step_amount = (int)(NUDGE_STEPS * (1.0f + boost));

    // === İlk dürtme sadece 1 defa yapılır ===
    if (!initial_kick_done) {
        int initial_steps = NUDGE_STEPS * 3;
        step_motor(initial_steps, true);
        sleep_ms(100);
        step_motor(initial_steps / 2, false);
        initial_kick_done = true;
        sleep_ms(200);
    }

    // === Swing-up algoritması ===
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
    }

    // === Core0'a veri aktarımı ===
    monitor_data.theta = theta;
    monitor_data.omega = omega;
    monitor_data.x = x;
    monitor_data.swing_phase = swing_phase;

    sleep_ms(WAIT_MS);
}
