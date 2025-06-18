#include "1_hardware_configuration.h"
#include "2_buzzer.h"

#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include <stdio.h>


// Mario temasındaki notaların frekansları (Hz)
#define NOTE_E7  2637
#define NOTE_G7  3136
#define NOTE_A7  3520
#define NOTE_B7  3951
#define NOTE_E8  5274
#define NOTE_FS7 2960
#define NOTE_D7  2349
#define NOTE_C7  2093
#define REST     0

// Mario temasının notaları ve süreleri (ms cinsinden)
int melody[] = {
    NOTE_E7, NOTE_E7, REST, NOTE_E7,
    REST, NOTE_C7, NOTE_E7, REST,
    NOTE_G7, REST, REST, NOTE_E8,
    REST, NOTE_E8, REST, NOTE_E8,
    REST, NOTE_E8
};

int duration[] = {
    150, 150, 150, 150,
    150, 150, 150, 150,
    300, 150, 150, 150,
    150, 150, 150, 150, 
    150, 600
};
/*
// PWM kanalını ayarla
void buzzer_init() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_enabled(slice_num, true);
}*/

// PWM ile belli frekansta ses çıkar
void play_tone(int frequency, int duration_ms) {
    if (frequency == REST) {
        sleep_ms(duration_ms);
        return;
    }

    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint clk = 125000000; // Pico clock
    uint wrap = clk / frequency - 1;

    pwm_set_wrap(slice, wrap);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(BUZZER_PIN), wrap / 2); // 50% duty
    pwm_set_enabled(slice, true);

    sleep_ms(duration_ms);
    pwm_set_enabled(slice, false);
    sleep_ms(20); // kısa ara
}

// Mario temasını sırayla çal
void buzzer_play_mario_theme() {
    printf("[debug] Buzzer_init...\n");
    int notes = sizeof(melody) / sizeof(melody[0]);
    for (int i = 0; i < notes; i++) {
        play_tone(melody[i], duration[i]);
    }    
    printf("[debug] Buzzer_init...Cleared\n");
}
