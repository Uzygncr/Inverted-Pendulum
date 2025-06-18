#include "1_hardware_configuration.h"
#include "2_blink.h"
#include "pico/cyw43_arch.h"


//=== blink_init ===//
void blink_init(){
    printf("[debug] Blink_init...\n");
    if (cyw43_arch_init()) {
        printf("Wifi module cannot be started.\n");
    }
    cyw43_arch_gpio_put(LED_PIN, false); // LED başlangıç durumu
    printf("[debug] Blink_init...Cleared\n");
}

//=== LED Blink Callback Function ===//
bool blink_callback(repeating_timer_t *rt) {
    static bool led_state = false;
    led_state = !led_state;
    cyw43_arch_gpio_put(LED_PIN, led_state);
    return true; // Timer goes on
}
