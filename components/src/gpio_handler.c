#include "gpio_handler.h"

#include "sys_config.h"

#include "driver/gpio.h"

#include "esp_mac.h"
/**
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

bool is_blink_led = 1;
/**
 * Configure the ESP32 gpios (LED & button );
 */
void led_on()
{
    gpio_set_level(LED_BUILDING, 1);
}
void led_off()
{
    gpio_set_level(LED_BUILDING, 0);
}
void gpios_setup()
{
    esp_rom_gpio_pad_select_gpio(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    /**
     * Configure the GPIO LED BUILDING
     */
    gpio_reset_pin(LED_BUILDING);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED_BUILDING, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_BUILDING, 0);

    xTaskCreate(blink_led_task, "blink_led_task", 1024 * 2, NULL, 10, NULL);
}
void blink_led_task(void *pvParameters)
{
    while (1)
    {
        if (is_blink_led)
        {
            led_on();
            vTaskDelay(200 / portTICK_PERIOD_MS);
            led_off();
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
void blink_led()
{
    is_blink_led = 1;
}
void stop_blink_led()
{
    is_blink_led = 0;
}