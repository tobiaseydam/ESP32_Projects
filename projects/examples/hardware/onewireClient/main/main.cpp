#include "esp_log.h"
#include "tey_onewire.hpp"

extern "C" {
    void app_main(void);
}

void app_main(void){
    onewire_config *conf = new onewire_config(GPIO_NUM_17);

    onewire_client *cl = new onewire_client(conf);
    cl->continous_read();

    while(1){
        vTaskDelay(pdMS_TO_TICKS(5*1000));
        ESP_LOGI("onewireClient", "Temperatur: %.2f °C", conf->get_device(0)->get_temperature());

    }
}