#include "esp_log.h"

extern "C" {
    void app_main(void);
}

static constexpr char *TAG = (char*)"emptyProject";

void app_main(void){
    ESP_LOGI(TAG, "hello World");
}