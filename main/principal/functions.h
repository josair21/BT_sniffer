#ifndef FUNCTIONS_H
#define FUNCTIONS_H
extern String devices[];
extern String deviceToConnect;
extern int iDevice;
extern bool isDone;
void begin_ble();
void search_ble();
void clear_ble();
bool connect_ble(int a);
void download_ble();
void disconnect_ble();
#endif
