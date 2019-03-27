#ifndef TEY_MQTT_HPP
    #define TEY_MQTT_HPP

#include "mqtt_client.h"
#include "esp_err.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string>

#define MQTT_CONNECTED_BIT       BIT0
#define MQTT_DISCONNECTED_BIT    BIT1

class mqtt_client_config{
    private:
        static constexpr char *TAG = (char*)"mqtt_client_config";
    protected:
        std::string host;
        uint16_t port;
        EventGroupHandle_t event_group = NULL;
    public:
        mqtt_client_config();

        void set_host(std::string value) { host = value; };
        std::string get_host() { return host; };

        void set_port(uint16_t value) { port = value; };
        uint16_t get_port() { return port; };

        EventGroupHandle_t get_event_group() { return event_group; };
};

class mqtt_client{
    private:
        static constexpr char *TAG = (char*)"mqtt_client";
    protected:
        mqtt_client_config *cfg;
        esp_mqtt_client_handle_t client;
        static esp_err_t event_handler(esp_mqtt_event_handle_t event);
    public:
        mqtt_client(mqtt_client_config *mqtt_cfg);
        void connect();
};

#endif