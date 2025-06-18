#ifndef TALKER_H
#define TALKER_H

void talker_send(const char *msg);
void talker_sendln(const char *msg);  // Sonuna \r\n ekler
void talker_send_connect_ack(void);   // Connect mesajı

#endif
