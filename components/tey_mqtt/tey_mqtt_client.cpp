#include "tey_mqtt.hpp"
#include "esp_log.h"
#include "esp_event_loop.h"

#include <string.h>


mqtt_client_config::mqtt_client_config(){ 
    event_group = xEventGroupCreate();
}

void mqtt_client_config::add_subscription(std::string topic, mqtt_subscription_callback_t callback, uint8_t qos){
    subs[topic] = callback;
    esp_mqtt_client_subscribe(client, topic.c_str(), qos);
}

mqtt_subscription_callback_t mqtt_client_config::get_subscription_callback(std::string topic){
    return subs[topic];
}

esp_err_t mqtt_client::event_handler(esp_mqtt_event_handle_t event){
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:{
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_client_config* conf = (mqtt_client_config*) event->user_context;
            xEventGroupClearBits(conf->get_event_group(), 0xFF);
            xEventGroupSetBits(conf->get_event_group(), MQTT_CONNECTED_BIT);
            break;
        }
        case MQTT_EVENT_DISCONNECTED:{
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            mqtt_client_config* conf = (mqtt_client_config*) event->user_context;
            xEventGroupClearBits(conf->get_event_group(), 0xFF);
            xEventGroupSetBits(conf->get_event_group(), MQTT_DISCONNECTED_BIT);
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
            mqtt_client_config* conf = (mqtt_client_config*) event->user_context;
            
            char *c_topic = new char[event->topic_len];
            strncpy(c_topic, event->topic, event->topic_len);
            
            mqtt_subscription_callback_t cb = conf->get_subscription_callback(c_topic);
            if(cb != NULL){
                cb(event);
            }
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
    config.user_context = cfg;
    cfg->set_client(esp_mqtt_client_init(&config));
}

void mqtt_client::connect(){
    esp_mqtt_client_start(cfg->get_client());
}

int mqtt_client::publish(std::string topic, std::string payload, uint8_t qos, bool retain){
    ESP_LOGD(TAG, "Publishing Topic: %s", topic.c_str());
    return esp_mqtt_client_publish(cfg->get_client(), topic.c_str(), payload.c_str(), 0, qos, retain);
}
