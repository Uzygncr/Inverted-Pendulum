#ifndef MOTOR_H
#define MOTOR_H

#include "pico/stdlib.h"

extern volatile int position;

// Fonksiyon Prototipleri
void motor_init();

void motor_backoff_from_sensor();

void motor_calibration_phase_1(void);
void motor_calibration_phase_2(void);
void motor_calibration_phase_3(void);


void motor_disable(void);       // Kalibrasyon sonunda motoru durdurur
void motor_enable(void);


void step_motor(int steps, bool direction);
void motor_step_once(bool direction);
int get_position();
float get_speed();
float get_acceleration();

#endif // MOTOR_H
