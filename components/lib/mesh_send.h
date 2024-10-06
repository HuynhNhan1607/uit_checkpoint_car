#pragma once
#include <stdbool.h>

void send_mesh(char *data_t);
void send_setup_msg(char *topic, char *data);
bool send_connect_msg(void);
bool send_pincap_layer(int voltage, int layer, char *ID);
void send_disconnect_msg(char *macID);
void send_sensor_msg(void);