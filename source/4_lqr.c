#include "4_lqr.h"
#include "2_encoder.h"
#include "2_motor.h"
#include "math.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define K_THETA     5.0f
#define K_THETA_DOT 1.0f
#define K_X         0.1f
#define K_X_DOT     0.1f






#define LQR_ANGLE_THRESHOLD_DEG 20.0f
#define MAX_STEPS 400
#define MIN_EFFECTIVE_STEPS 5

extern volatile struct {
    float theta;
    float omega;
    float x;
    int swing_phase;
} monitor_data;

void lqr_control() {
    float theta = encoder_get_angle();       // derece [-180, 180]
    float omega = angular_velocity;          // derece/s
    float x = get_position();                // motor pozisyonu
    float x_dot = get_speed();               // motor hızı

    // Açıyı normalize et → dik konum 0°
    theta -= 180.0f;
    if (theta < -180.0f) theta += 360.0f;
    if (theta > 180.0f)  theta -= 360.0f;

    // LQR sadece ±20° içinde aktif
    if (fabsf(theta) > LQR_ANGLE_THRESHOLD_DEG) {
        motor_disable();  // güvenli çıkış
        return;
    } else {motor_enable();}

    // LQR kontrol sinyali hesapla
    float u = -(
        K_THETA     * theta +
        K_THETA_DOT * omega +
        K_X         * x +
        K_X_DOT     * x_dot
    );

    int steps = (int)(fabsf(u));
    if (steps > MAX_STEPS) steps = MAX_STEPS;

    if (steps > MIN_EFFECTIVE_STEPS) {
        bool direction = (u > 0);
        step_motor(steps, direction);
    }

    // Monitör verisi (isteğe bağlı)
    monitor_data.theta = theta;
    monitor_data.omega = omega;
    monitor_data.x = x;
    monitor_data.swing_phase = 99;  // 99 = LQR (swing phase dışında)
    printf("[LQR DEBUG] θ=%.2f, ω=%.2f, x=%.2f, x_dot=%.2f → u=%.2f → steps=%d\n",
       theta, omega, x, x_dot, u, steps);


    
    
    sleep_ms(1);
}
