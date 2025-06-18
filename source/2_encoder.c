#include "2_encoder.h"
#include "2_motor.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

#include <stdio.h>
#include <math.h>


// Global Variables
volatile int latest_encoder_ticks = 0;
volatile float last_angle = 0.0;
volatile float angular_velocity = 0.0;
volatile float angular_acceleration = 0.0;
absolute_time_t last_update_time;
volatile int encoder_ticks = 0;
volatile int last_a = 0;
volatile bool data_ready_to_print = false;

//=== Encoder ISR ===//
void encoder_isr(uint gpio, uint32_t events) {
    int current_a = gpio_get(ENCODER_A_PIN);
    int current_b = gpio_get(ENCODER_B_PIN);

    if (current_a != last_a) {
        encoder_ticks += (current_a == current_b) ? 1 : -1; // Yön tayini

        // Encoder ticks'i sınırla
        if (encoder_ticks >= TICKS_PER_REVOLUTION) {
            encoder_ticks -= TICKS_PER_REVOLUTION; // 360'a ulaştı, sıfırdan başla
        } else if (encoder_ticks < 0) {
            encoder_ticks += TICKS_PER_REVOLUTION; // 0'ın altına düştü, 360'a dön
        }
    }

    last_a = current_a; // A sinyalini güncelle
}

// Encoder ticks değerini döndürür
int encoder_get_ticks() {
    return encoder_ticks;
}

float encoder_get_angle() {
    float angle = ((float)encoder_ticks / TICKS_PER_REVOLUTION) * 360.0f;

    // -180° ile +180° aralığına normalize et
    angle = fmodf(angle + 180.0f, 360.0f);
    if (angle < 0) angle += 360.0f;
    angle -= 180.0f;

    return angle;
}


//=== Encoder'i sıfırlar ===//
void encoder_reset() {
    encoder_ticks = 0;
}

//=== Encoder callback ===//
bool update_encoder_callback(repeating_timer_t *rt) {
    latest_encoder_ticks = encoder_get_ticks();
    return true; // Timer devam etsin
}

bool print_data_callback(repeating_timer_t *rt) {
    data_ready_to_print = true;
    return true;
}

// Açısal hız ve ivme hesaplama fonksiyonu
void calculate_angular_data() {
    // Şu anki zaman
    absolute_time_t now = get_absolute_time();
    int64_t delta_time_us = absolute_time_diff_us(last_update_time, now);
    float delta_time_s = delta_time_us / 1000000.0;

    // Mevcut açı
    float current_angle = latest_encoder_ticks * DEGREE_PER_TICK;

    // Açısal hız hesaplama
    float delta_angle = current_angle - last_angle;
    float new_angular_velocity = delta_angle / delta_time_s;

    // Açısal ivme hesaplama
    angular_acceleration = (new_angular_velocity - angular_velocity) / delta_time_s;

    // Güncelle
    angular_velocity = new_angular_velocity;
    last_angle = current_angle;
    last_update_time = now;
    
}

// Ark uzunluğu, hızı ve ivmesi ile açı, açısal hız ve ivmeyi hesaplama ve yazdırma
void print_data() {
    // Mevcut pozisyon, hız ve ivme değerlerini al
    float position = get_position();
    float speed = get_speed();
    float acceleration = get_acceleration();

    // Ark verilerini hesapla
    float position_rad = position * (M_PI / 180.0); // Dereceden radyana
    float speed_rad = speed * (M_PI / 180.0);       // Dereceden radyana
    float acceleration_rad = acceleration * (M_PI / 180.0); // Dereceden radyana

    float arc_length = position_rad * RADIUS_MM; // mm
    float arc_velocity = speed_rad * RADIUS_MM; // mm/s
    float arc_acceleration = acceleration_rad * RADIUS_MM; // mm/s²

    // Açısal verileri hesapla
    calculate_angular_data();

    // UART üzerinden sabit genişlikte yazdır
    printf("|| %-15s | %-17s | %-20s |\n", "Değer", "Ark Verisi", "Açısal Verisi");
    printf("||----------------|-------------------|--------------------|\n");
    printf("| %-14s | %9.2f mm      | %9.2f derece   |\n", "Uzunluk", arc_length, last_angle);
    printf("| %-15s | %9.2f mm/s    | %9.2f derece/s |\n", "Hız", arc_velocity, angular_velocity);
    printf("| %-15s | %9.1f mm/s²   | %9.1f derece/s²|\n", "İvme", arc_acceleration, angular_acceleration);
    printf("\n"); // Boşluk bırak
}
