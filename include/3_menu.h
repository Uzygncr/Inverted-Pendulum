#ifndef MENU_H
#define MENU_H

#include <stdbool.h>


extern volatile bool mode_auto ;
void menu_init(void);                    // Başlangıç için çağrılır
void menu_handle_command(const char *cmd); // USB'den gelen komut işlenir
void menu_update(void);                  // Süreye bağlı geçişleri tetikler



#endif
