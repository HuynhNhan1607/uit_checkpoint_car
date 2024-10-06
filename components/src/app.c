#include "driver/gpio.h"
#include "mesh_handler.h"
#include "mesh_send.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
/**
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
/*
 */
#include "cJSON.h"

#include "sys_config.h"

static const char *TAG = "Task Call";

static adc_cali_handle_t adc2_cali_handle;

void task_send_bat_capacity(void *pvParameter)
{
    //-------------ADC2 Init---------------//
    adc_oneshot_unit_handle_t adc2_handle;
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &adc2_handle));

    //-------------ADC2 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_0, &config));

    //-------------ADC2 Calibration Init---------------//
    // adc_cali_handle_t adc2_cali_handle = NULL;
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_2,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    esp_err_t ret;
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &adc2_cali_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "Calibration failed");
    }

    // adc_cali_line_fitting_config_t cali_config = {
    //     .unit_id = ADC_UNIT_2,
    //     .atten = ADC_ATTEN_DB_12,
    //     .bitwidth = ADC_BITWIDTH_12,
    // };
    // // Tạo cấu hình hiệu chuẩn ADC
    // esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, &adc2_cali_handle);
    // ESP_ERROR_CHECK(ret);

    // // Cấu hình kênh ADC2
    // ret = adc2_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_12);
    // ESP_ERROR_CHECK(ret);

    int voltage;
    int adc_raw;
    while (1)
    {
        int sum_adc_raw = 0;
        for (int i = 0; i < 64; i++)
        {
            ESP_ERROR_CHECK(adc_oneshot_read(adc2_handle, ADC_CHANNEL_0, &adc_raw));
            sum_adc_raw += adc_raw;
        }
        adc_raw = sum_adc_raw / 64; // Calculate average from 64 samples

        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc2_cali_handle, adc_raw, &voltage));

        printf("Raw: %d, voltage: %u mV\n", adc_raw * 2, voltage * 2);
        while (!send_pincap_layer(voltage * 2, 0, NODE_ID))
        {
            vTaskDelay(5000 / portTICK_PERIOD_MS); // Thử lại sau 5 giây nếu gửi thất bại
        }
        vTaskDelay(300000 / portTICK_PERIOD_MS); // Gửi mỗi 5 phút
    }
}

void task_create_send_bat_capacity()
{

    if (xTaskCreate(task_send_bat_capacity, "task_send_bat_capacity", 1024 * 5, NULL, 1, NULL) != pdPASS)
    {
#if DEBUG
        ESP_LOGI(TAG, "ERROR - task_mesh_rx NOT ALLOCATED :/\r\n");
#endif
        return;
    }
}
