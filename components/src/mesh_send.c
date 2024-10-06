
#include "esp_log.h"
#include "esp_mesh.h"
#include "esp_mac.h"
#include "cJSON.h"
#include "nvs_handler.h"
#include "sys_config.h"
#include "mqtt_handler.h"
#include "mesh_handler.h"
#include "mesh_send.h"
#include "http_client_handler.h"
#include "gpio_handler.h"
#include <stdint.h>

static uint8_t tx_buf[TX_SIZE] = {
    0,
};
static uint8_t rx_buf[RX_SIZE] = {
    0,
};

extern mesh_addr_t route_table[CONFIG_MESH_ROUTE_TABLE_SIZE];
extern char mesh_root_addr[20];

extern nvs_handle my_handle;
extern uint16_t *max_range_extern;

static const char *TAG_SEND = "MESH_SEND";
void send_mesh(char *data_t)
{
    mesh_data_t data;
    int route_table_size;
    data.data = tx_buf;
    data.size = TX_SIZE;
    data.proto = MESH_PROTO_JSON;
    snprintf((char *)tx_buf, TX_SIZE, data_t);
    data.size = strlen((char *)tx_buf) + 1;

    esp_mesh_get_routing_table((mesh_addr_t *)&route_table,
                               CONFIG_MESH_ROUTE_TABLE_SIZE * 6, &route_table_size);
    char mac_str[30];
    for (int i = 0; i < route_table_size; i++)
    {
        sprintf(mac_str, MACSTR, MAC2STR(route_table[i].addr));
        if (strcmp(mesh_root_addr, mac_str) != 0)
        {
            esp_err_t err;
            err = esp_mesh_send(&route_table[i], &data, MESH_DATA_P2P, NULL, 0);
            if (err)
            {
#if DEBUG
                ESP_LOGI(TAG_SEND, "ERROR : Sending Mesh Message!\r\n");
#endif
            }
            else
            {
#if DEBUG
                ESP_LOGI(TAG_SEND, "ROOT (%s) sends (%s) to NON-ROOT (%s)\r\n", mesh_root_addr, tx_buf, mac_str);
#endif
            }
            vTaskDelay(200);
        }
    }
}
//
/*Respond mqtt msg Root to Nonroot*/
void send_setup_msg(char *topic, char *data)
{
    cJSON *root;
    if (strcmp(topic, "range") == 0)
    {
        root = cJSON_Parse(data);
        char *ID = cJSON_GetObjectItem(root, "node")->valuestring;
        if (strcmp(ID, NODE_ID) == 0)
        {
            *max_range_extern = cJSON_GetObjectItem(root, "range")->valueint;
#if DEBUG
            ESP_LOGI(TAG_SEND, "%d", *max_range_extern);
#endif
#if DEBUG
            ESP_LOGI(TAG_SEND, "It root");
#endif
        }
        else
        {
            cJSON_AddStringToObject(root, "Topic", topic);
            char *rendered = cJSON_Print(root);
            send_mesh(rendered);
        }
    }
    else if (strcmp(topic, "check") == 0)
    {
        root = cJSON_Parse(data);
        char *ID = cJSON_GetObjectItem(root, "node")->valuestring;
        if (strcmp(ID, NODE_ID) == 0)
        {
            mqtt_app_publish("ESP-connect", NODE_ID);
        }
        else
        {
            cJSON_AddStringToObject(root, "Topic", topic);
            char *rendered = cJSON_Print(root);
            send_mesh(rendered);
        }
    }
    else if (strcmp(topic, "ip") == 0)
    {
        root = cJSON_Parse(data);
        char *ip_setup = cJSON_GetObjectItem(root, "ip")->valuestring;
        cJSON_AddStringToObject(root, "Topic", topic);
        char *rendered = cJSON_Print(root);
        send_mesh(rendered);
        save_ip(&my_handle, ip_setup);
    }
}
//
bool send_connect_msg()
{
    if (esp_mesh_is_root())
    {
        // mqtt_app_publish("ESP-connect", NODE_ID);
        return true;
    }
    else
    {
        uint8_t chipid[20];
        char mac_str[30];
        esp_efuse_mac_get_default(chipid);
        snprintf(mac_str, sizeof(mac_str), "" MACSTR "", MAC2STR(chipid));

        cJSON *root;
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "Topic", "Connect-Mesh");
        cJSON_AddStringToObject(root, "ID", NODE_ID);
        cJSON_AddStringToObject(root, "MAC", mac_str);
        char *rendered = cJSON_Print(root);
        snprintf((char *)tx_buf, TX_SIZE, rendered);

        mesh_data_t data;
        data.data = tx_buf;
        data.size = strlen((char *)tx_buf) + 1;
        esp_err_t err = esp_mesh_send(NULL, &data, MESH_DATA_P2P, NULL, 0);
        if (err)
        {
            return false;
#if DEBUG
            ESP_LOGI(TAG_SEND, "ERROR : Sending Connect Message!\r\n");
#endif
        }
        else
        {
            return true;
#if DEBUG
            ESP_LOGI(TAG_SEND, "\r\nNON-ROOT sends (%s) (%s) to ROOT (%s)\r\n", mac_str, tx_buf, mesh_root_addr);
#endif
        }
        return false;
    }
}
//
bool send_pincap_layer(int voltag_SENDe, int layer, char *ID)
{
    char mac_str[30];
    esp_err_t err;
    mesh_data_t data;
    data.data = tx_buf;
    data.size = TX_SIZE;
    data.proto = MESH_PROTO_JSON;

    if (layer == 0)
    {
        layer = esp_mesh_get_layer();
    }
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "pin", voltag_SENDe);
    cJSON_AddNumberToObject(root, "layer", layer);
    cJSON_AddStringToObject(root, "node", ID);
    if (esp_mesh_is_root())
    {
        char *rendered = cJSON_Print(root);
        mqtt_app_publish("ESP-cap-layer", rendered);
        return 1;
    }
    else
    {
        cJSON_AddStringToObject(root, "Topic", "Send-pin-layer");
        char *rendered = cJSON_Print(root);
        snprintf((char *)tx_buf, TX_SIZE, rendered);
        data.size = strlen((char *)tx_buf) + 1;
        err = esp_mesh_send(NULL, &data, MESH_DATA_P2P, NULL, 0);
        if (err)
        {
#if DEBUG
            ESP_LOGI(TAG_SEND, "ERROR : Sending Pin Cap Message!\r\n");
#endif
            return 0;
        }
        else
        {
            uint8_t chipid[20];
            esp_efuse_mac_get_default(chipid);
            snprintf(mac_str, sizeof(mac_str), "" MACSTR "", MAC2STR(chipid));
#if DEBUG
            ESP_LOGI(TAG_SEND, "\r\nNON-ROOT sends (%s) (%s) to ROOT (%s)\r\n", mac_str, tx_buf, mesh_root_addr);
#endif
            return 1;
        }
        return 0;
    }
}
//
void send_disconnect_msg(char *macID)
{
#if SEND_DISCONNECT
    if (esp_mesh_is_root())
    {
        for (int i = 0; i < lengthOfActiveNode; i++)
        {
            if (strcmp(macID, activeNode[i].mac) == 0)
            {
                mqtt_app_publish("ESP-disconnect", activeNode[i].id);
            }
        }
    }
    else
    {
        uint8_t chipid[20];
        char mac_str[30];
        esp_efuse_mac_get_default(chipid);
        snprintf(mac_str, sizeof(mac_str), "" MACSTR "", MAC2STR(chipid));
        cJSON *root;
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "Topic", "Disconnect-Mesh");
        cJSON_AddStringToObject(root, "MAC", macID);
        char *rendered = cJSON_Print(root);
        snprintf((char *)tx_buf, TX_SIZE, rendered);

        mesh_data_t data;
        data.data = tx_buf;
        data.size = strlen((char *)tx_buf) + 1;
        esp_err_t err = esp_mesh_send(NULL, &data, MESH_DATA_P2P, NULL, 0);
        if (err)
        {
#if DEBUG
            ESP_LOGI(TAG_SEND, "ERROR : Sending Disconnected Message!\r\n");
#endif
        }
        else
        {
#if DEBUG
            ESP_LOGI(TAG_SEND, "\r\nNON-ROOT sends (%s) (%s) to ROOT (%s)\r\n", mac_str, tx_buf, mac_address_root_str);
#endif
        }
    }
#endif
}
//
void send_sensor_msg()
{
    char mac_str[30];
    esp_err_t err;
    mesh_data_t data;
    data.data = tx_buf;
    data.size = TX_SIZE;
    data.proto = MESH_PROTO_JSON;
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "Topic", "Send-Data");
    cJSON_AddStringToObject(root, "Data", NODE_ID);
    cJSON_AddNumberToObject(root, "Tick", takeTick());
    char *rendered = cJSON_Print(root);
    if (esp_mesh_is_root())
    {
        // mqtt_app_publish("ESP-send", NODE_ID);
        task_send_data(rendered);
    }
    else
    {
        snprintf((char *)tx_buf, TX_SIZE, rendered);
        data.size = strlen((char *)tx_buf) + 1;
        err = esp_mesh_send(NULL, &data, MESH_DATA_P2P, NULL, 0);
        if (err)
        {
            ESP_LOGI(TAG_SEND, "ERROR : Sending Sensor Message!\r\n");
        }
        else
        {
            uint8_t chipid[20];
            esp_efuse_mac_get_default(chipid);
            snprintf(mac_str, sizeof(mac_str), "" MACSTR "", MAC2STR(chipid));
#if DEBUG
            ESP_LOGI(TAG_SEND, "\r\nNON-ROOT sends (%s) (%s) to ROOT (%s)\r\n", mac_str, tx_buf, mesh_root_addr);
#endif
        }
    }
}

void task_send_data(char *post_data)
{
    xTaskCreate(http_post, "post_rest_function", 1024 * 4, post_data, 2, NULL);
}
void task_mesh_rx(void *pvParameter)
{
    esp_err_t err;
    mesh_addr_t from;

    mesh_data_t data;
    data.data = rx_buf;
    data.size = RX_SIZE;

    char mac_address_str[30];
    int flag = 0;

    ESP_LOGI(TAG_SEND, "Rx start");

    for (;;)
    {
        data.size = RX_SIZE;
        if (esp_mesh_is_root())
        { // Is it root node? Then turn on the led building
            led_on();
        }
        /**
         * Waits for message reception
         */
        err = esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);
        if (err != ESP_OK || !data.size)
        {
#if DEBUG
            ESP_LOGI(TAG_SEND, "err:0x%x, size:%d", err, data.size);
#endif
            continue;
        }
        char myJson[100];
        snprintf(myJson, 100, (char *)data.data);

        cJSON *root = cJSON_Parse(myJson);
        char *topic = cJSON_GetObjectItem(root, "Topic")->valuestring;
        /**
         * Is it routed for ROOT Node?
         */
        if (esp_mesh_is_root())
        {
            //**ROOT handle message
            if (strcmp(topic, "Send-Data") == 0)
            {
                char *nodeData = cJSON_GetObjectItem(root, "Data")->valuestring;
                // mqtt_app_publish("ESP-send", nodeData);
                task_send_data(myJson);
#if DEBUG
                ESP_LOGI(TAG_SEND, "NON-ROOT(MAC:%s)- Node %s: %s, ", mac_address_str, topic, nodeData);

#endif
            }
            else if (strcmp(topic, "Connect-Mesh") == 0)
            {
                char *id = cJSON_GetObjectItem(root, "ID")->valuestring;
                mqtt_app_publish("ESP-connect", id);
#if SEND_DISCONNECT
                char *mac = cJSON_GetObjectItem(root, "MAC")->valuestring;
                for (int i = 0; i <= lengthOfActiveNode; i++)
                {
                    if (i == lengthOfActiveNode)
                    {
                        activeNode[lengthOfActiveNode].id = id;
                        activeNode[lengthOfActiveNode++].mac = mac;
                        break;
                    }
                    if (lengthOfActiveNode != 0)
                        if (strcmp(mac, activeNode[i].mac) == 0)
                        {
                            activeNode[i].id = id;
                            break;
                        }
                    // ESP_LOGI(TAG_SEND, "Active node HERE");
                    // ESP_LOGI(TAG_SEND, "Active node%s",activeNode[i].mac);
                }
#endif
                snprintf(mac_address_str, sizeof(mac_address_str), "" MACSTR "", MAC2STR(from.addr));
#if DEBUG
                ESP_LOGI(TAG_SEND, "NON-ROOT(MAC:%s)- Node %s: %s, ", mac_address_str, topic, id);
                ESP_LOGI(TAG_SEND, "Tried to publish %s", id);
#endif
            }
            else if (strcmp(topic, "Send-pin-layer") == 0)
            {
                char *ID = cJSON_GetObjectItem(root, "node")->valuestring;
                int voltage = cJSON_GetObjectItem(root, "pin")->valueint;
                int layer = cJSON_GetObjectItem(root, "layer")->valueint;
                send_pincap_layer(voltage, layer, ID);
            }
#if SEND_DISCONNECT
            else if (strcmp(topic, "Disconnect-Mesh") == 0)
            {
                char *macID = cJSON_GetObjectItem(root, "MAC")->valuestring;
#if DEBUG
                ESP_LOGI(TAG_SEND, "NON-ROOT(MAC:%s)- Node %s: %s, ", mac_address_str, topic, macID);
                ESP_LOGI(TAG_SEND, "Tried to publish %s", macID);
#endif
                send_disconnect_msg(macID);
            }
#endif
#if DEBUG
            ESP_LOGI(TAG_SEND, "ROOT(MAC:%s) - Msg: %s, ", mesh_root_addr, data.data);
            ESP_LOGI(TAG_SEND, "send by NON-ROOT: %s\r\n", mac_address_str);
#endif
        }

        else
        {
            uint8_t mac_address[10];
            esp_efuse_mac_get_default(mac_address);
            snprintf(mac_address_str, sizeof(mac_address_str), "" MACSTR "", MAC2STR(mac_address));
            if (strcmp(topic, "range") == 0)
            {
                *max_range_extern = cJSON_GetObjectItem(root, "range")->valueint;
#if DEBUG
                ESP_LOGI(TAG_SEND, "MQTT send %s", myJson);
#endif
            }
            else if (strcmp(topic, "check") == 0)
            {
                char *node = cJSON_GetObjectItem(root, "node")->valuestring;
                if (strcmp(node, NODE_ID) == 0)
                {
                    while (!send_connect_msg())
                        vTaskDelay(200 / portTICK_PERIOD_MS);
                }
            }
            else if (strcmp(topic, "ip") == 0)
            {
                char *ip_setup = cJSON_GetObjectItem(root, "ip")->valuestring;
                save_ip(&my_handle, ip_setup);
            }
#if DEBUG
            ESP_LOGI(TAG_SEND, "NON-ROOT(MAC:%s)- Msg: %s, ", mac_address_str, (char *)data.data);
            snprintf(mac_address_str, sizeof(mac_address_str), "" MACSTR "", MAC2STR(from.addr));
            ESP_LOGI(TAG_SEND, "send by ROOT: %s\r\n", mac_address_str);
#endif
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}
void task_app_create(void)
{
#if DEBUG
    ESP_LOGI(TAG_SEND, "task_app_create() called");
    if (esp_mesh_is_root())
    {
        ESP_LOGI(TAG_SEND, "ROOT NODE\r\n");
    }
    else
    {
        ESP_LOGI(TAG_SEND, "CHILD NODE\r\n");
    }
#endif
    /**
     * Creates a Task to receive message;
     */
    if (xTaskCreatePinnedToCore(task_mesh_rx, "task_mesh_rx", 1024 * 5, NULL, 2, NULL, 0) != pdPASS)
    {
#if DEBUG
        ESP_LOGI(TAG_SEND, "ERROR - task_mesh_rx NOT ALLOCATED :/\r\n");
#endif
        return;
    }
}
