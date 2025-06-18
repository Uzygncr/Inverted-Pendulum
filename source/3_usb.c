#include "3_usb.h"
#include "pico/stdlib.h"

static volatile char last_command = 0;
static volatile bool command_available = false;

void usb_init(void) {
    stdio_init_all();
    sleep_ms(500); // USB bağlantısı otursun
}

void usb_check_command(void) {
    int ch = getchar_timeout_us(0);
    if (ch != PICO_ERROR_TIMEOUT) {
        last_command = (char)ch;
        command_available = true;
    }
}

bool usb_command_received(void) {
    return command_available;
}

char usb_get_last_command(void) {
    return last_command;
}

void usb_clear_last_command(void) {
    command_available = false;
}
