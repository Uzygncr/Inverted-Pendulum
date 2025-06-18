
#include "2_encoder.h"
#include "3_monitor.h"

#include "pico/stdlib.h"

#include <stdio.h>
#include <math.h>

/*#define ENCODER_TICKS_PER_REV 1000
#define DEGREE_PER_TICK (360.0 / ENCODER_TICKS_PER_REV)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float last_angle = 0.0f;
static float angular_velocity = 0.0f;
static float angular_acceleration = 0.0f;
static absolute_time_t last_update_time;

void calculate_angular_data(void) {
    absolute_time_t now = get_absolute_time();
    int64_t delta_time_us = absolute_time_diff_us(last_update_time, now);
    float delta_time_s = delta_time_us / 1e6f;

    int ticks = encoder_get_ticks();
    float current_angle = ticks * DEGREE_PER_TICK;
    float delta_angle = current_angle - last_angle;
    float new_velocity = delta_angle / delta_time_s;

    angular_acceleration = (new_velocity - angular_velocity) / delta_time_s;
    angular_velocity = new_velocity;
    last_angle = current_angle;
    last_update_time = now;
}

void print_encoder_info(void) {
    printf("| Pos  | %7.2f deg |\n", last_angle);
    printf("| Vel  | %7.2f deg/s |\n", angular_velocity);
    printf("| Acc  | %7.2f deg/s\u00b2 |\n\n", angular_acceleration);
}*/