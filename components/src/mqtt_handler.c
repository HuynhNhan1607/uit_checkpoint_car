#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_tls.h"

/**
 * Standard configurations loaded
 */
#include "sys_config.h"
#include "app.h"

#include "mqtt_client.h"
#include "mqtt_handler.h"
#include "mesh_handler.h"
#include "mesh_send.h"

static const char *TAG_MQTT = "MQTT_Handler";
static esp_mqtt_client_handle_t s_client = NULL;

extern char ip[50];

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
        if (esp_mqtt_client_subscribe(s_client, "range", 0) < 0 || esp_mqtt_client_subscribe(s_client, "refresh", 0) < 0 || esp_mqtt_client_subscribe(s_client, "check", 0) < 0 || esp_mqtt_client_subscribe(s_client, "ip", 0) < 0)
        {
            // Disconnect to retry the subscribe after auto-reconnect timeout
            esp_mqtt_client_disconnect(s_client);
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
        ESP_LOGI(TAG_MQTT, "TOPIC=%.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG_MQTT, "DATA=%.*s", event->data_len, event->data);
        char topic[50];
        snprintf(topic, 100, "%.*s", event->topic_len, event->topic);
        send_setup_msg(topic, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
    mqtt_event_handler_cb(event_data);
}

bool mqtt_app_publish(char *topic, char *publish_string)
{
    if (s_client)
    {
        int msg_id = esp_mqtt_client_publish(s_client, topic, publish_string, 0, 1, 0);
        ESP_LOGI(TAG_MQTT, "sent publish returned msg_id=%d", msg_id);
        return true;
    }
    return false;
}

void mqtt_app_start(void)
{
    char broker_host[70] = BROKER_HOST;
    // sprintf(broker_host, "mqtt://%s:1883", ip);
    ESP_LOGI(TAG_MQTT, "Broker: %s", broker_host);
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker_host,
        .broker.address.port = 1883,
        // Public after disconnect 30s
        .session.last_will.topic = "ESP-disconnect",
        .session.last_will.msg = NODE_ID,
        .session.last_will.msg_len = 0,
        .session.last_will.qos = 1,
        .session.last_will.retain = 1,
        .session.keepalive = 60};

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, s_client);
    esp_mqtt_client_start(s_client);
}
