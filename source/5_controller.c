#include "5_controller.h"
#include "4_swing_up.h"
#include "4_lqr.h"
#include "2_encoder.h"
#include "pico/stdlib.h"
#include <math.h>
#include <stdio.h>

#define LQR_ANGLE_THRESHOLD 20.0f
#define LQR_SPEED_THRESHOLD 30.0f

typedef enum {
    MODE_SWING_UP,
    MODE_LQR
} ControllerMode;

static ControllerMode mode = MODE_SWING_UP;

void controller_run() {
    float theta = encoder_get_angle() - 180.0f;

    // Normalize açı [-180, 180]
    if (theta < -180.0f) theta += 360.0f;
    if (theta > 180.0f)  theta -= 360.0f;

    float omega = angular_velocity;

    bool in_lqr_range = fabsf(theta) < LQR_ANGLE_THRESHOLD && fabsf(omega) < LQR_SPEED_THRESHOLD;

    if (mode == MODE_SWING_UP && in_lqr_range) {
        mode = MODE_LQR;
        //printf("[CTRL] LQR'ye geçiliyor\n");
    }

    else if (mode == MODE_LQR && !in_lqr_range) {
        mode = MODE_SWING_UP;
        //printf("[CTRL] Swing-up'a geri dönülüyor\n");
    }

    // Aktif moda göre çağır
    if (mode == MODE_LQR)
        lqr_control();
    else
        swing_up_control();
}
