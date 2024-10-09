#include <stdio.h>
#include <string.h>
#include "esp_http_client.h"   // Thư viện cho HTTP client
#include "esp_log.h"           // Thư viện ghi log
#include "cJSON.h"             // Thư viện xử lý JSON
#include "freertos/FreeRTOS.h" // FreeRTOS cho việc quản lý tác vụ
#include "freertos/task.h"     // Các hàm về task trong FreeRTOS
#include "esp_system.h"        // Thư viện hệ thống ESP
#include "esp_err.h"           // Xử lý lỗi
#include "esp_event.h"         // Thư viện sự kiện hệ thống ESP
#include "esp_mac.h"
#include "esp_timer.h"

#include "app.h"
#include "http_client_handler.h"
#include "sys_config.h"

long long Tick = 0;
long long previousTick = 0;
extern bool is_tick_be_get;

static const char *TAG_HTTP = "HTTP_Client";

unsigned long IRAM_ATTR millis() { return (unsigned long)(esp_timer_get_time() / 1000ULL); }

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static int output_len; // Stores number of bytes read
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG_HTTP, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG_HTTP, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG_HTTP, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG_HTTP, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        if (output_len == 0 && evt->user_data)
        {
            // we are just starting to copy the output data into the use
            memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
        }
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            int copy_len = 0;
            if (evt->user_data)
            {
                // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                if (copy_len)
                {
                    memcpy(evt->user_data + output_len, evt->data, copy_len);
                }
            }
            output_len += copy_len;
        }
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;
    default:
        break;
    }
    return ESP_OK;
}

long long takeTick()
{
    return Tick + (millis() - previousTick) / 10;
}
void http_get_tick()
{
    char local_response_buffer[2048] = {0};

    esp_http_client_config_t config = {
        .url = "http://192.168.7.11:3001/getTick",
        .method = HTTP_METHOD_GET,
        .user_data = local_response_buffer,
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG_HTTP, "HTTPS Status = %d, content_length = %" PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
        cJSON *root = cJSON_Parse((char *)local_response_buffer);
        Tick = cJSON_GetObjectItem(root, "tick")->valueint;
        previousTick = millis();
        Tick += 27;
        is_tick_be_get = true;
    }
    else
    {
        ESP_LOGE(TAG_HTTP, "Error perform http request %s", esp_err_to_name(err));
    }
    printf("%s\n", local_response_buffer);
    esp_http_client_cleanup(client);
}

void http_post(void *post_data)
{
    esp_http_client_config_t config_post = {
        .url = SERVER_IP "/send-data",
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = _http_event_handler};

    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_HTTP, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}