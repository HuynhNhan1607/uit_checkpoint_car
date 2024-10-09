| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |


Below is short explanation of remaining files in the project folder.

```
│   CMakeLists.txt
│   README.md
│   sdkconfig
│   sdkconfig.old
│   
├───.devcontainer
│       
├───.vscode
│
├───components
│   │   CMakeLists.txt
│   │
│   ├───lib
│   │       app.h
│   │       gpio_handler.h
│   │       http_client_handler.h
│   │       mesh_handler.h
│   │       mesh_send.h
│   │       mqtt_handler.h
│   │       nvs_handler.h
│   │       sys_config.h
│   │       vl53l0x_handler.h
│   │       wifi_handler.h
│   │
│   ├───pololu_vl53l0x
│   │       i2c.cpp
│   │       i2c.h
│   │       vl53l0x.cpp
│   │       vl53l0x.h
│   │
│   └───src
│           app.c
│           gpio_handler.c
│           http_client_handler.c
│           mesh_handler.c
│           mesh_send.c
│           mqtt_handler.c
│           nvs_handler.c
│           vl53l0x_handler.cpp
│           wifi_handler.c
│
└───main
        CMakeLists.txt
        main.cpp            This is the file you are currently reading
```

