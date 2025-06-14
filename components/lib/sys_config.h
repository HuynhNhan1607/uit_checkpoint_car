
#define SSID "UIT_CAR_RACING_2023"
#define PASS "sinhvien_CEEC"
#define BROKER_HOST "mqtt://192.168.7.11:1883"
#define SERVER_IP "http://192.168.7.11:3001"
#define MESH_ID_DEFINE {0x77, 0x77, 0x77, 0x77, 0x77, 0x77}
#define NODE_ID "8"

#define LED_BUILDING (27)
#define GPIO_OUTPUT_PIN_SEL (1ULL << LED_BUILDING)

#define BUTTON_PIN (0)
#define GPIO_INPUT_PIN_SEL (1ULL << BUTTON)

#define DEBUG 1
#define SEND_DISCONNECT 0

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define CONFIG_MESH_CHANNEL 0
