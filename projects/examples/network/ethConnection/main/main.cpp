#include "esp_log.h"

extern "C" {
    void app_main(void);
}

void app_main(void){
    ESP_LOGI("test", "hello World");
}