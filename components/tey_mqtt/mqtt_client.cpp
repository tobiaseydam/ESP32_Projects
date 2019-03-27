#include "tey_mqtt.hpp"
#include "esp_log.h"
#include "esp_event_loop.h"

#include <string.h>


mqtt_client_config::mqtt_client_config(){ 
    event_group = xEventGroupCreate();
}

esp_err_t mqtt_client::event_handler(esp_mqtt_event_handle_t event){
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:{
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            break;
        }
        case MQTT_EVENT_DISCONNECTED:{
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        }
        case MQTT_EVENT_SUBSCRIBED:{
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
            break;
        }
        case MQTT_EVENT_UNSUBSCRIBED:{
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED");
            break;
        }
        case MQTT_EVENT_PUBLISHED:{
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
            break;
        }
        case MQTT_EVENT_DATA:{
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            break;
        }
        case MQTT_EVENT_ERROR:{
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        }
    }
    return ESP_OK;
}

mqtt_client::mqtt_client(mqtt_client_config *mqtt_cfg){
    cfg = mqtt_cfg;

    esp_mqtt_client_config_t config;
    memset(&config, 0, sizeof(config)); 
    config.host = cfg->get_host().c_str();
    config.port = cfg->get_port();
    config.event_handle = event_handler;
    client = esp_mqtt_client_init(&config);
}

void mqtt_client::connect(){
    esp_mqtt_client_start(client);
}