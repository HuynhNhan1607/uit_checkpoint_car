#include <stdio.h>
#include "sys_config.h"
extern "C" {
#include "nvs_handler.h"
#include "mesh_handler.h"
#include "mesh_send.h"
#include "wifi_handler.h"
#include "http_client_handler.h"
#include "app.h"
#include "gpio_handler.h"
}
#include "vl53l0x_handler.h"
#include "vl53l0x.h"


extern "C" {
void app_main(void);
}

VL53L0X vl53l0x_c;
vl53l0x_t vl53l0x = {
    .i2c = &i2c,
    .sensor = &vl53l0x_c,
    .max_range = 250,
    .range = 0};
uint16_t *max_range_extern = &vl53l0x.max_range;

extern nvs_handle my_handle;


void app_main(void)
{  
    nvs_init();
    save_pass(&my_handle, SSID, PASS);
    task_create_send_bat_capacity();
    gpios_setup();
    wifi_mesh_start();
    sensor_start(&vl53l0x);
}
