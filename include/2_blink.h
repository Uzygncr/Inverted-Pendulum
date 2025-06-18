#ifndef BLINK_H
#define BLINK_H

#include <stdbool.h>
#include "1_hardware_configuration.h"
#include "pico/stdlib.h"


//=== Function call prototypes ===//
void blink_init();
bool blink_callback(repeating_timer_t *rt);

#endif // BLINK_H
