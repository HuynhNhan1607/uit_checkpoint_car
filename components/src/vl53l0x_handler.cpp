#include "vl53l0x_handler.h"

#include "sys_config.h"

/**
 * Logs
 */
#include "esp_log.h"

#include "vl53l0x.h"

#include "i2c.h"
extern "C" {
#include "app.h"

#include "gpio_handler.h"
#include "mesh_handler.h"
#include "mesh_send.h"
}
static const char *TAG = "vl53l0x: ";

uint16_t sensor_read(vl53l0x_t* vl53l0x)
{
    uint16_t range_measure = vl53l0x->sensor->readRangeSingleMillimeters();
            if (vl53l0x->sensor->timeoutOccurred()) {
                ESP_LOGI( TAG, "TIMEOUT\r\n" );  
                }
    // vTaskDelay(15 / portTICK_RATE_MS);
    // range_measure += vl53l0x->sensor->readRangeSingleMillimeters();
    // if (vl53l0x->sensor->timeoutOccurred()) {
    //             ESP_LOGI( TAG, "TIMEOUT\r\n" );  
    //             }
    // return range_measure/2;
    return range_measure;
}
void sensor_task(void* pvParameters){
    vl53l0x_t* vl53l0x = (vl53l0x_t*) pvParameters;
    bool flag = 1;
    int filter = 0;
    while (1) {
        // range = sensor_read();
        // sensor->readRangeContinuousMillimeters
        vl53l0x->range = sensor_read(vl53l0x);
        // ESP_LOGI( TAG, "Range: %d\r\n", range );  
        while (vl53l0x->range <= vl53l0x->max_range && vl53l0x->range > 20 ){
            // ESP_LOGI( TAG, "Range: %d\r\n", vl53l0x->range );  
            
            // ESP_LOGI( TAG, "Console log %d", filter );  
            if (++filter > 3){
                #if END_NODE
                if(filter > 25 && flag)
                {
                    send_sensor_msg(); 
                    flag = 0;
                }
                #elif STOP_STATION_NODE
                if(filter > 43 && flag)
                {
                    send_sensor_msg(); 
                    flag = 0;
                }
                #else
                if (flag) 
                {    
                    ESP_LOGI( TAG, "Console log %d", filter );  
                    send_sensor_msg(); 
                    flag = 0;
                }
                #endif
                led_on();
            }
            vl53l0x->range = sensor_read(vl53l0x);    
            vTaskDelay(25 / portTICK_PERIOD_MS);
        }
        filter = 0;
        if (!flag){
            led_off();
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        flag = 1;
        vTaskDelay(25 / portTICK_PERIOD_MS);
    }
}
void sensor_start(vl53l0x_t * vl53l0x){
    vl53l0x->i2c->init();
    vl53l0x->sensor->setTimeout(500);
    if (!vl53l0x->sensor->init()) {
        ESP_LOGW( TAG, "Failed to detect and initialize sensor!\r\n" );  
        
        while (1) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        return;
    }
    
    // sensor->startContinuous(100);
    vl53l0x->sensor->setMeasurementTimingBudget(25000);
    vl53l0x->sensor->setSignalRateLimit(0.3);
    #if defined LONG_RANGE
        // lower the return signal rate limit (default is 0.25 MCPS)
        vl53l0x->sensor.setSignalRateLimit(0.1);
        // increase laser pulse periods (defaults are 14 and 10 PCLKs)
        vl53l0x->sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
        vl53l0x->sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
    #endif

    #if defined HIGH_SPEED
        // reduce timing budget to 20 ms (default is about 33 ms)
        vl53l0x->sensor.setMeasurementTimingBudget(20000);
    #elif defined HIGH_ACCURACY
        // increase timing budget to 200 ms
        vl53l0x->sensor.setMeasurementTimingBudget(200000);
    #endif
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    if( xTaskCreate( sensor_task, "sensor_task", 1024 * 8, vl53l0x, 2, NULL) != pdPASS )
    {
        #if DEBUG
        ESP_LOGI( TAG, "ERROR - sensor_task NOT ALLOCATED :/\r\n" );  
        #endif
        return;   
    }   
}