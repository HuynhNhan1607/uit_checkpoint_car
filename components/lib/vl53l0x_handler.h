#pragma once

#include "vl53l0x.h"
#include "i2c.h"

struct vl53l0x_t
{   
    _i2c* i2c;
    VL53L0X* sensor;
    uint16_t max_range;
    uint16_t range;
} typedef vl53l0x_t;

uint16_t sensor_read(vl53l0x_t *);
void sensor_task(void* pvParameters);
void sensor_start(vl53l0x_t *);
