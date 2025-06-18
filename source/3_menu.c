#include "2_lcd.h"
#include "2_motor.h"
#include "3_calibration.h"
#include "3_menu.h"
#include "3_talker.h"

#include "pico/stdlib.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    MENU_CONNECTION_WAIT,
    MENU_CONNECTED,
    MENU_CHECKBOXES,
    MENU_SECURITY_OK,
    MENU_CALIBRATION,
    MENU_START_STOP_PRE,
    MENU_START_STOP,
    MENU_MODE_SELECT,
    MENU_AUTO_MODE,
    MENU_MANUAL_OPTIONS,
    MENU_FOLLOW_MODE,
    MENU_GOTO_MODE,
    MENU_RTH_MODE
} MenuState;

static MenuState current_state = MENU_CONNECTION_WAIT;
static absolute_time_t state_time;

static bool safe = false;
static bool damage = false;
static bool calibration = false;
static bool start_stop = false;

volatile bool mode_auto = false;
static bool mode_manual = false;

void menu_init() {
    current_state = MENU_CONNECTION_WAIT;
    state_time = get_absolute_time();
    lcd_clear();
    lcd_write_line(0, "Baglanti");
    lcd_write_line(1, "Bekleniyor...");
}

static void update_lcd() {
    lcd_clear();

    switch (current_state) {
        case MENU_CONNECTION_WAIT:
            lcd_write_line(0, "Baglanti");
            lcd_write_line(1, "Bekleniyor...");
            break;

        case MENU_CONNECTED:
            lcd_write_line(0, "Baglanti");
            lcd_write_line(1, "Saglandi");
            break;

        case MENU_CHECKBOXES: {
            char l0[17], l1[17];
            snprintf(l0, 17, "Safe:   %s", safe ? "[X]" : "[ ]");
            snprintf(l1, 17, "Damage: %s", damage ? "[X]" : "[ ]");
            lcd_write_line(0, l0);
            lcd_write_line(1, l1);
            break;
        }

        case MENU_SECURITY_OK:
            lcd_write_line(0, "Guvenlik");
            lcd_write_line(1, "Saglandi");
            break;

        case MENU_CALIBRATION:
            lcd_write_line(0, "Calibration:");
            lcd_write_line(1, calibration ? "[X]" : "[ ]");
            break;

        case MENU_START_STOP_PRE:
            lcd_write_line(0, "Start/Stop");
            lcd_write_line(1, "Enabled");
            break;

        case MENU_START_STOP:
            lcd_write_line(0, "Start/Stop:");
            lcd_write_line(1, start_stop ? "[X]" : "[ ]");
            break;

        case MENU_MODE_SELECT: {
            char l1[17];
            snprintf(l1, 17, "[%c] Auto [%c] Man", mode_auto ? 'X' : ' ', mode_manual ? 'X' : ' ');
            lcd_write_line(0, "Advance Mode");
            lcd_write_line(1, l1);
            break;
        }

        case MENU_AUTO_MODE:
            lcd_write_line(0, "Auto Mode Aktif");
            lcd_write_line(1, "Bekleniyor...");
            break;

        case MENU_MANUAL_OPTIONS:
            lcd_write_line(0, "Manual Menu:");
            lcd_write_line(1, "F G R Seciniz");
            break;

        case MENU_FOLLOW_MODE:
            lcd_write_line(0, "Follow Modu");
            lcd_write_line(1, "Takip Ediliyor");
            break;

        case MENU_GOTO_MODE:
            lcd_write_line(0, "GoTo Modu");
            lcd_write_line(1, "Pozisyona Gidis");
            break;

        case MENU_RTH_MODE:
            lcd_write_line(0, "RTH Modu");
            lcd_write_line(1, "Eve Donus");
            break;
    }

    state_time = get_absolute_time();
}

void menu_handle_command(const char *cmd) {
    printf("[CMD] Komut: %s | State: %d\n", cmd, current_state);

    if (strcmp(cmd, "Connect") == 0 && current_state == MENU_CONNECTION_WAIT) {
        current_state = MENU_CONNECTED;
        update_lcd();
        talker_send_connect_ack();
    }

    else if (strcmp(cmd, "NotConnect") == 0) {
        safe = false;
        damage = false;
        calibration = false;
        start_stop = false;
        mode_auto = false;
        mode_manual = false;
        current_state = MENU_CONNECTION_WAIT;
        update_lcd();
    }

    else if (strcmp(cmd, "Safe") == 0 && current_state == MENU_CHECKBOXES) {
        safe = !safe;
        update_lcd();
    }

    else if (strcmp(cmd, "NotSafe") == 0) {
        safe = false;
        current_state = MENU_CHECKBOXES;
        update_lcd();
    }

    else if (strcmp(cmd, "Damage") == 0 && current_state == MENU_CHECKBOXES) {
        damage = !damage;
        update_lcd();
    }

    else if (strcmp(cmd, "NotDamage") == 0) {
        damage = false;
        current_state = MENU_CHECKBOXES;
        update_lcd();
    }

    else if (strcmp(cmd, "Calibration") == 0 && current_state == MENU_CALIBRATION) {
        motor_arm_calibration();
        calibration = true;
        current_state = MENU_START_STOP_PRE;
        update_lcd();
    }

    else if (strcmp(cmd, "NotCalibration") == 0) {
        calibration = false;
        current_state = MENU_CALIBRATION;
        update_lcd();
    }

    else if (strcmp(cmd, "Start") == 0 && current_state == MENU_START_STOP) {
        motor_enable();
        start_stop = true;
        update_lcd();
    }

    else if (strcmp(cmd, "Stop") == 0) {
        motor_disable();
        start_stop = false;
        current_state = MENU_START_STOP;
        update_lcd();
    }

    else if (strcmp(cmd, "AutoMode") == 0 && current_state == MENU_MODE_SELECT) {
        mode_auto = true;
        mode_manual = false;
        current_state = MENU_AUTO_MODE;
        update_lcd();
    }

    else if (strcmp(cmd, "NotAutoMode") == 0) {
    mode_auto = false;

    // Menüye geri dön
    current_state = MENU_MODE_SELECT;
    update_lcd();
    }


    else if (strcmp(cmd, "ManualControl") == 0 && current_state == MENU_MODE_SELECT) {
        mode_manual = true;
        mode_auto = false;
        current_state = MENU_MANUAL_OPTIONS;
        update_lcd();
    }

    else if (strncmp(cmd, "FollowAngle ", 12) == 0 && current_state == MENU_FOLLOW_MODE) {
    int target_deg = atoi(cmd + 12);
    int target_steps = (target_deg - 180) * 32;

    int current = get_position();
    int steps = abs(target_steps - current);
    bool direction = (target_steps > current);

    printf("[Follow] Hedef: %d°, Mevcut: %d, Adım: %d, Yön: %s\n",
           target_deg, current, steps, direction ? "ileri" : "geri");

    if (steps > 0) {
        step_motor(steps, direction);
    }
    }



    else if (strncmp(cmd, "GoTo ", 5) == 0) {
    current_state = MENU_MANUAL_OPTIONS;
    update_lcd();

    int target_deg = atoi(cmd + 5);  // örn. "GoTo 261"
    int target_steps = (target_deg - 180) * 32;

    int current = get_position();
    int steps = abs(target_steps - current);
    bool direction = (target_steps > current);

    printf("[GoTo] Açı: %d°, Hedef: %d, Mevcut: %d, Adım: %d, Yön: %s\n",
           target_deg, target_steps, current, steps, direction ? "ileri" : "geri");

    if (steps > 0) {
        step_motor(steps, direction);
    } else {
        printf("[GoTo] Zaten hedef pozisyondasın.\n");
    }

    // Hedefe ulaştıktan sonra eski menüye dön
    current_state = MENU_MANUAL_OPTIONS;
    update_lcd();
    }



    else if (strcmp(cmd, "RTH") == 0) {
    // Menü geçişini yap
    current_state = MENU_MANUAL_OPTIONS;
    update_lcd();

    // Pozisyon ve yön hesapla
    int current = get_position();
    int target = 0;

    int steps = abs(current - target);
    bool direction = (target > current);

    printf("[RTH] Mevcut: %d → 0 | Adım: %d | Yön: %s\n",
        current, steps, direction ? "ileri" : "geri");

    if (steps > 0) {
        step_motor(steps, direction);
    } else {
        printf("[RTH] Zaten evdesin.\n");
    }

    // Eve dönüş tamam → eski menüye dön
    current_state = MENU_MANUAL_OPTIONS;
    update_lcd();
    }




    else if (strcmp(cmd, "Reset") == 0) {
        safe = damage = calibration = start_stop = false;
        mode_auto = mode_manual = false;
        current_state = MENU_CHECKBOXES;
        update_lcd();
    }
}

void menu_update(void) {
    int64_t elapsed = absolute_time_diff_us(state_time, get_absolute_time());

    switch (current_state) {
        case MENU_CONNECTED:
            if (elapsed > 500 * 1000) {
                current_state = MENU_CHECKBOXES;
                update_lcd();
            }
            break;

        case MENU_CHECKBOXES:
            if (safe && damage) {
                current_state = MENU_SECURITY_OK;
                update_lcd();
            }
            break;

        case MENU_SECURITY_OK:
            if (elapsed > 250 * 1000) {
                current_state = MENU_CALIBRATION;
                update_lcd();
            }
            break;

        case MENU_CALIBRATION:
            if (calibration && elapsed > 250 * 1000) {
                current_state = MENU_START_STOP_PRE;
                update_lcd();
            }
            break;

        case MENU_START_STOP_PRE:
            if (elapsed > 250 * 1000) {
                current_state = MENU_START_STOP;
                update_lcd();
            }
            break;

        case MENU_START_STOP:
            if (start_stop) {
                current_state = MENU_MODE_SELECT;
                update_lcd();
            }
            break;

        default:
            break;
    }
}
