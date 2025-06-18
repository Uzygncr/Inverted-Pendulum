#ifndef USB_H
#define USB_H

#include <stdbool.h>

void usb_init(void);                     // USB bağlantısını başlat
void usb_check_command(void);            // Komut var mı diye kontrol et
bool usb_command_received(void);         // Komut geldiyse true döner
char usb_get_last_command(void);         // Gelen son karakteri verir
void usb_clear_last_command(void);       // Komutu sıfırlar

#endif // USB_H
