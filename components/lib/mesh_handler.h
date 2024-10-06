#pragma once

#define MESH_CHANNEL 6

#define CONFIG_MESH_MAX_LAYER 2
#define CONFIG_MESH_AP_AUTHMODE 3 // WIFI_AUTH_WPA2_PSK
#define CONFIG_MESH_AP_CONNECTIONS 6
#define CONFIG_MESH_NON_MESH_AP_CONNECTIONS 2
#define CONFIG_MESH_ROUTE_TABLE_SIZE 50
#define CONFIG_MESH_AP_PASSWD "12345678"
#define RX_SIZE (1500)
#define TX_SIZE (1460)

/* void send_setup_msg(char *, char *);

bool send_connect_msg(void);

bool send_pincap_layer(int, int, char *);

void send_disconnect_msg(char *);

void send_sensor_msg(void); */

void task_send_data(char *);

void task_mesh_rx(void *);

void task_app_create(void);

void wifi_mesh_start();