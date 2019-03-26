#include "esp_log.h"
#include "network.hpp"

extern "C" {
    void app_main(void);
}

void app_main(void){
    ESP_LOGI("ethConnection", "configuring...");
    eth_config* conf = new eth_config();

    ESP_LOGI("ethConnection", "establishing ETH-Connection...");
    network_adapter* nw = new network_adapter(conf);

    nw->connect();

}