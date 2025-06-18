#include "1_hardware_configuration.h"
#include "2_hall_sensor.h"
#include "hardware/gpio.h"

#include <stdio.h>  // debug için

bool hall_sensor_triggered(void) {
    int a = gpio_get(HALL_A_PIN);
    int d = gpio_get(HALL_D_PIN);
    //printf("[debug HALL] A=%d, D=%d → %s\n", a, d, (a == 0 && d == 1) ? "TRIGGERED" : "not triggered");
    return (a == 0 && d == 1);
}
