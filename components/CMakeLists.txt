idf_component_register(SRCS "src/nvs_handler.c" "src/mesh_handler.c" "src/vl53l0x_handler.cpp"
                            "src/wifi_handler.c" "src/http_client_handler.c" "src/app.c"
                            "src/mqtt_handler.c" "src/gpio_handler.c" "src/mesh_send.c"
                            "pololu_vl53l0x/i2c.cpp" "pololu_vl53l0x/vl53l0x.cpp"
                    INCLUDE_DIRS "./lib" "./pololu_vl53l0x"
                    REQUIRES freertos esp_wifi esp_event esp_wifi esp_http_client nvs_flash esp_system log json driver esp_timer mqtt esp_adc)
