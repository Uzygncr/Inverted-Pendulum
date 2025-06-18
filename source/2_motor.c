#include "1_hardware_configuration.h"
#include "2_motor.h"
#include "2_hall_sensor.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>



// Global pozisyon değişkeni
volatile int position = 0;
static float motor_speed = 0.0;         // Motorun mevcut hızı (adım/s)
static float motor_acceleration = 0.0; // Motorun mevcut ivmesi (adım/s²)
static absolute_time_t last_update_time; // Son hız/ivme güncelleme zamanı
static int steps_between_edges = 0;

// === Motor_init ===//
void motor_init() {
    gpio_init(MOTOR_EN_PIN);
    gpio_set_dir(MOTOR_EN_PIN, GPIO_OUT);
    gpio_put(MOTOR_EN_PIN, 0);  // 0 = Enable (aktif)

    gpio_init(MOTOR_DIR_PIN);
    gpio_set_dir(MOTOR_DIR_PIN, GPIO_OUT);
    gpio_put(MOTOR_DIR_PIN, 0);

    gpio_init(MOTOR_STEP_PIN);
    gpio_set_dir(MOTOR_STEP_PIN, GPIO_OUT);
    gpio_put(MOTOR_STEP_PIN, 0);

    // İlk güncelleme zamanını ayarla
    last_update_time = get_absolute_time();
}

// Motorun pozisyonunu döndürür
int get_position() {
    return position;
}

// Motorun hızını döndürür
float get_speed() {
    return motor_speed;
}

// Motorun ivmesini döndürür
float get_acceleration() {
    return motor_acceleration;
}

// Motorun hız ve ivme değerlerini günceller
static void update_motion_data(int steps) {
    // Şu anki zaman
    absolute_time_t now = get_absolute_time();
    int64_t delta_time_us = absolute_time_diff_us(last_update_time, now);

    // Zaman farkı (saniye cinsinden)
    float delta_time_s = delta_time_us / 1000000.0;

    // Hız hesaplama
    float delta_position = (float)steps; // Hareket edilen adım sayısı
    float new_speed = delta_position / delta_time_s;

    // İvme hesaplama
    motor_acceleration = (new_speed - motor_speed) / delta_time_s;

    // Güncel hız
    motor_speed = new_speed;

    // Zamanı güncelle
    last_update_time = now;
}

// Motoru çalıştırır
void step_motor(int steps, bool direction) {
    
    gpio_put(MOTOR_DIR_PIN, !direction); // Motor yönünü ayarla

    // Hareket algoritması
    float current_speed = MIN_SPEED;
    float acceleration = 0;
    float jerk = 0;
    float obtained_speed = MAX_SPEED;

    float accel_distance = steps * MAX_ACCEL_DISTANCE_RATIO;
    if (steps < 2000) {
        accel_distance = steps * MIN_ACCEL_DISTANCE_RATIO;
    }

    float decel_start = steps - accel_distance;
    float accel_time = cbrtf((6.0f * accel_distance) / MAX_JERK);
    float time_step = accel_time / accel_distance;

    for (int i = 0; i < steps; i++) {
        int delay_us = 700000 / current_speed;

        // Adım sinyali gönder
        gpio_put(MOTOR_STEP_PIN, true);
        sleep_us(7); // Pulse genişliği
        gpio_put(MOTOR_STEP_PIN, false);
        sleep_us(delay_us - 10);

        // Pozisyon güncelle
        if (direction) {
            position++;
        } else {
            position--;
        }

        // Hız ve ivme güncellemesi
        if (i < accel_distance) {
            if (i < accel_distance / 2) {
                jerk = MAX_JERK;
                acceleration += jerk * time_step;
                if (acceleration > MAX_ACCELERATION) acceleration = MAX_ACCELERATION;
            } else {
                jerk = -MAX_JERK;
                acceleration += jerk * time_step;
                if (acceleration < 0) acceleration = 0;
            }
            current_speed += acceleration * time_step;
            if (current_speed > obtained_speed) current_speed = obtained_speed;
        } else if (i >= decel_start) {
            if (i < decel_start + accel_distance / 2) {
                jerk = MAX_JERK;
                acceleration += jerk * time_step;
                if (acceleration > MAX_ACCELERATION) acceleration = MAX_ACCELERATION;
            } else {
                jerk = -MAX_JERK;
                acceleration += jerk * time_step;
                if (acceleration < 0) acceleration = 0;
            }
            current_speed -= acceleration * time_step;
            if (current_speed < MIN_SPEED) current_speed = MIN_SPEED;
        }
    }

    // Hareketin sonunda hız ve ivme güncellenir
    update_motion_data(steps);
}


void motor_step_once(bool direction) {
    gpio_put(MOTOR_DIR_PIN, direction);     // Yönü ayarla
    gpio_put(MOTOR_STEP_PIN, 1);            // STEP sinyalini başlat
    sleep_us(10);                            // Pulse genişliği
    gpio_put(MOTOR_STEP_PIN, 0);            // STEP sinyalini kapat
    sleep_us(100); 

    // İsteğe bağlı: pozisyon güncellemesi
    if (direction) {
        position++;
    } else {
        position--;
    }
}

// === Motor Pasif === //
void motor_disable(void) {
    gpio_put(MOTOR_EN_PIN, 1); // Motoru devre dışı bırak (disable)
}

// === Motor Aktif === //
void motor_enable(void) {
    gpio_put(MOTOR_EN_PIN, 0); // Enable motor
}

// === Güç kolu kalibrasyonu === //
void motor_calibration_phase_1(void) {
    motor_enable();
    gpio_put(MOTOR_DIR_PIN, true); // ileri yön

    float current_speed = MIN_SPEED;
    float acceleration = 0;
    float jerk = MAX_JERK;
    float time_step = 0.0005f;

    const float max_speed = 1000.0f;
    const float max_accel = 1000.0f;

    bool slowing_down = false;

    while (true) {
        int delay_us = 1000000 / current_speed;
        if (delay_us < 200) delay_us = 200;

        // step pulse
        gpio_put(MOTOR_STEP_PIN, 1); sleep_us(10);
        gpio_put(MOTOR_STEP_PIN, 0); sleep_us(delay_us - 10);

        // Sensör algılandıysa yavaşlama başlasın
        if (hall_sensor_triggered() && !slowing_down) {
            slowing_down = true;
            jerk = -MAX_JERK;  // ters jerk → deceleration
        }

        // Eğer sensör görüldü ve artık çıkış kenarına gelindiyse → bitir
        if (slowing_down && !hall_sensor_triggered()) {
            break;  // çıkış kenarı bulundu
        }

        // İvme ve hız güncelle
        acceleration += jerk * time_step;
        if (acceleration < 0) acceleration = 0;
        if (acceleration > max_accel) acceleration = max_accel;

        current_speed += acceleration * time_step;
        if (current_speed < MIN_SPEED) current_speed = MIN_SPEED;
        if (current_speed > max_speed) current_speed = max_speed;
    }

    //printf("[debug  edge] Calibrasyon_phase_1...Cleared\n");
}

void motor_calibration_phase_2(void) {
    gpio_put(MOTOR_DIR_PIN, false);  // geri yön

    float current_speed = MIN_SPEED;
    float acceleration = 0;
    float jerk = MAX_JERK;
    float time_step = 0.0005f;

    const float max_speed = 1000.0f;
    const float max_accel = 1000.0f;

    bool slowing_down = false;
    steps_between_edges = 0;

    while (true) {
        int delay_us = 1000000 / current_speed;
        if (delay_us < 200) delay_us = 200;

        gpio_put(MOTOR_STEP_PIN, 1); sleep_us(10);
        gpio_put(MOTOR_STEP_PIN, 0); sleep_us(delay_us - 10);

        if (!slowing_down && hall_sensor_triggered()) {
            slowing_down = true;
            jerk = -MAX_JERK; // Yavaşlamaya geç
        }

        if (slowing_down && !hall_sensor_triggered()) {
            break; // çıkış kenarı tespit edildi
        }

        if (slowing_down) {
            steps_between_edges++;
        }

        // hız/ivme güncelle
        acceleration += jerk * time_step;
        if (acceleration > max_accel) acceleration = max_accel;
        if (acceleration < 0) acceleration = 0;

        current_speed += acceleration * time_step;
        if (current_speed > max_speed) current_speed = max_speed;
        if (current_speed < MIN_SPEED) current_speed = MIN_SPEED;
    }
}

void motor_calibration_phase_3(void) {
    gpio_put(MOTOR_DIR_PIN, true); // ileri yön

    int half_steps = steps_between_edges / 2;

    float current_speed = MIN_SPEED;
    float acceleration = 0;
    float jerk = MAX_JERK;
    float time_step = 0.0005f;

    const float max_speed = 1000.0f;
    const float max_accel = 1000.0f;

    for (int i = 0; i < half_steps; i++) {
        int delay_us = 1000000 / current_speed;
        if (delay_us < 200) delay_us = 200;

        gpio_put(MOTOR_STEP_PIN, 1); sleep_us(10);
        gpio_put(MOTOR_STEP_PIN, 0); sleep_us(delay_us - 10);

        // smooth yavaşlama için jerk negatif yapılıyor (yarı yol sonuna doğru)
        if (i > half_steps / 2) {
            jerk = -MAX_JERK;
        }

        acceleration += jerk * time_step;
        if (acceleration > max_accel) acceleration = max_accel;
        if (acceleration < 0) acceleration = 0;

        current_speed += acceleration * time_step;
        if (current_speed > max_speed) current_speed = max_speed;
        if (current_speed < MIN_SPEED) current_speed = MIN_SPEED;
    }
}

void motor_backoff_from_sensor(void) {
    if (!hall_sensor_triggered()) return;  // gerek yoksa çık

    gpio_put(MOTOR_DIR_PIN, false); // geri yön

    const int total_steps = 500;
    float current_speed = MIN_SPEED;
    float acceleration = 0;
    float jerk = MAX_JERK;
    float time_step = 0.0005f;

    const float max_speed = 1000.0f;
    const float max_accel = 1000.0f;

    for (int i = 0; i < total_steps; i++) {
        int delay_us = 1000000 / current_speed;
        if (delay_us < 200) delay_us = 200;

        gpio_put(MOTOR_STEP_PIN, 1); sleep_us(10);
        gpio_put(MOTOR_STEP_PIN, 0); sleep_us(delay_us - 10);

        // Hız profilini uygula (yumuşak artış sonra azalma)
        if (i < total_steps / 2) {
            jerk = MAX_JERK;
        } else {
            jerk = -MAX_JERK;
        }

        acceleration += jerk * time_step;
        if (acceleration > max_accel) acceleration = max_accel;
        if (acceleration < 0) acceleration = 0;

        current_speed += acceleration * time_step;
        if (current_speed > max_speed) current_speed = max_speed;
        if (current_speed < MIN_SPEED) current_speed = MIN_SPEED;
    }
}

