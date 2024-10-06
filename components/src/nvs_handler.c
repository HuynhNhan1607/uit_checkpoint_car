#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "nvs_handler.h"

static const char *TAG_NVS = "NVS_Handler";
/*
    Khởi tạo vùng nhớ nvs
    Xóa vùng nhớ nếu không còn pages lưu trữ
*/
nvs_handle my_handle;

void nvs_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
}
/*

*/
void save_pass(nvs_handle *my_handle, const char *ssid, const char *pass)
{
    esp_err_t err = nvs_open("Wifi", NVS_READWRITE, my_handle);
    // Error Check
    if (err == ESP_OK)
    {
        err = nvs_set_str(*my_handle, "SSID", ssid);
        err = nvs_set_str(*my_handle, "PASS", pass);
        err = nvs_commit(*my_handle);
    }
    // Error Check
    nvs_close(*my_handle);
}
void save_ip(nvs_handle *my_handle, const char *ip)
{
    esp_err_t err = nvs_open("Wifi", NVS_READWRITE, my_handle);
    // Error Check
    err = nvs_set_str(*my_handle, "IP", ip);
    printf(TAG_NVS, (err != ESP_OK) ? "Add ip to nvs failed!\n" : "Add ip to nvs done\n");
    err = nvs_commit(*my_handle);
    // Error Check
    nvs_close(*my_handle);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();
}
void delete_part(nvs_handle *my_handle, const char *key) // Erase tung Key?
{
    nvs_open("Wifi", NVS_READWRITE, my_handle);
    // Error Check
    nvs_erase_key(*my_handle, key);

    nvs_commit(*my_handle);
    // Error Check
    nvs_close(*my_handle);
}

void delete_all(nvs_handle *my_handle)
{
    nvs_open("Wifi", NVS_READWRITE, my_handle);
    // Error Check
    nvs_erase_all(*my_handle);
    nvs_commit(*my_handle);
    // Error Check
    nvs_close(*my_handle);
}

/*
    Chưa hiểu lý do cần dùng
*/
char *get_part(nvs_handle *my_handle, const char *key)
{
    nvs_open("Wifi", NVS_READONLY, my_handle);
    size_t required_size = 0;
    nvs_get_str(*my_handle, key, NULL, &required_size);
    if (required_size == 0)
    {
        printf("Khong co gia tri cua: %s \n", key);
        return NULL;
    }
    else
    {
        char *value = malloc(required_size);
        nvs_get_str(*my_handle, key, value, &required_size);
        printf("NVS_%s: %s\n", key, value);
        nvs_close(*my_handle);
        return value;
    }
}
