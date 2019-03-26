#ifndef NETWORK_HPP
    #define NETWORK_HPP

#include "esp_err.h"
#include "esp_event.h"
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define START_BIT           BIT0
#define STOP_BIT            BIT1
#define CONNECTED_BIT       BIT2
#define DISCONNECTED_BIT    BIT3
#define GOT_IP_BIT          BIT4
#define LOST_IP_BIT         BIT5

class network_config{
    private:
        static constexpr char *TAG = (char*)"network_config";
    protected:
        std::string ip;
        EventGroupHandle_t event_group = NULL;
    public:
        network_config();

        std::string get_ip() { return ip; };
        void set_ip(std::string value) { ip = value; };

        EventGroupHandle_t get_event_group() { return event_group; };
};

class wifi_config: public network_config{
    private:
        static constexpr char *TAG = (char*)"wifi_config";
    public:
        wifi_config();
};

class eth_config: public network_config{
    private:
        static constexpr char *TAG = (char*)"eth_config";
    public:
        eth_config();
};

class network_adapter{
    private:
        static constexpr char *TAG = (char*)"network_adapter";
        TaskHandle_t run_task_handle = NULL;
    protected:
        network_config* conf;
        static esp_err_t event_handler(void *ctx, system_event_t *event);
        static void eth_gpio_config_rmii();
        static void run_task(void *param);
    public:
        network_adapter(wifi_config* config);
        network_adapter(eth_config* config);

        void connect();
};



#endif