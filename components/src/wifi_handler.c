#include "esp_wifi.h"
#include "sys_config.h"
#include <string.h>

#include "esp_log.h"
// Self Library
#include "nvs_handler.h"
#include "mesh_handler.h"
#include "wifi_handler.h"
#include "esp_mac.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

static const char *TAG_WIFI = "Wifi_Handler";

esp_netif_t *sta_netif = NULL;
EventGroupHandle_t s_wifi_event_group;

static const int CONNECTED_BIT = BIT0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG_WIFI, "Wifi Start");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
        ESP_LOGI(TAG_WIFI, "Retrying to connect to the AP");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
        ESP_LOGI(TAG_WIFI, "Connected to AP");
        // is_wifi_connect = true;
        break;
    case IP_EVENT_STA_GOT_IP:
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_WIFI, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    break;
    default:
        break;
    }
}

void wifi_init_sta()
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };
    memcpy((uint8_t *)&wifi_config.sta.ssid, SSID, strlen((char *)SSID));
    memcpy((uint8_t *)&wifi_config.sta.password, PASS, strlen((char *)PASS));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG_WIFI, "connected to ap SSID:%s PASS:%s",
                 SSID, PASS);
        is_wifi_connect = true;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG_WIFI, "Failed to connect to SSID:%s, PASS:%s",
                 SSID, PASS);
    }
    else
    {
        ESP_LOGE(TAG_WIFI, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}