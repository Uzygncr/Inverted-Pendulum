#ifndef ENCODER_H
#define ENCODER_H

#include "pico/stdlib.h"
#include "stdbool.h"
#include "1_hardware_configuration.h"

extern volatile bool data_ready_to_print;
extern volatile float angular_velocity;
extern volatile float angular_acceleration;
 

// Fonksiyon Prototipleri
void encoder_isr(uint gpio, uint32_t events);
void encoder_init();
int encoder_get_ticks();
void encoder_reset();
float encoder_get_angle();
bool update_encoder_callback(repeating_timer_t *rt); 
bool print_data_callback(repeating_timer_t *rt);
void print_data();
void calculate_angular_data();

#endif // ENCODER_H
