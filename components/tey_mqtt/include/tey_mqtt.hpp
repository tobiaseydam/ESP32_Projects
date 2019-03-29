#ifndef TEY_MQTT_HPP
    #define TEY_MQTT_HPP

#include "mqtt_client.h"
#include "esp_err.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string>
#include <map>
#define MQTT_CONNECTED_BIT       BIT0
#define MQTT_DISCONNECTED_BIT    BIT1

typedef void (*mqtt_subscription_callback_t)(esp_mqtt_event_handle_t event);

class mqtt_client_config{
    private:
        static constexpr char *TAG = (char*)"mqtt_client_config";
    protected:
        std::string host;
        uint16_t port;
        EventGroupHandle_t event_group = NULL;
        esp_mqtt_client_handle_t client;

       std::map<std::string, mqtt_subscription_callback_t> subs;
    public:
        mqtt_client_config();

        void set_host(std::string value) { host = value; };
        std::string get_host() { return host; };

        void set_port(uint16_t value) { port = value; };
        uint16_t get_port() { return port; };

        EventGroupHandle_t get_event_group() { return event_group; };

        void set_client(esp_mqtt_client_handle_t value) { client = value; }; 
        esp_mqtt_client_handle_t get_client() { return client; }; 

        void add_subscription(std::string topic, mqtt_subscription_callback_t callback, uint8_t qos);
        mqtt_subscription_callback_t get_subscription_callback(std::string topic);
};

class mqtt_client{
    private:
        static constexpr char *TAG = (char*)"mqtt_client";
    protected:
        mqtt_client_config *cfg;
        static esp_err_t event_handler(esp_mqtt_event_handle_t event);
    public:
        mqtt_client(mqtt_client_config *mqtt_cfg);
        void connect();

        int publish(std::string topic, std::string payload, uint8_t qos, bool retain);


};

#endif