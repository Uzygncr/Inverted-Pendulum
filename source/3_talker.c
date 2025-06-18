#include "3_talker.h"
#include <stdio.h>

void talker_send(const char *msg) {
    printf("%s", msg); // stdio USB üzerinden gönderir
}

void talker_sendln(const char *msg) {
    printf("%s\r\n", msg); // Satır sonu ekle
}

void talker_send_connect_ack(void) {
    talker_sendln("Connect established");
}
