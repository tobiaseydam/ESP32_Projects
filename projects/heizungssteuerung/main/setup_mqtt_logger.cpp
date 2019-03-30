#include "esp_log.h"
#include "tey_network.hpp"
#include "tey_mqtt.hpp"
#include "tey_onewire.hpp"
#include "tey_settings.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct{
    global_settings* sett;
    mqtt_client* mqtt_cl;
    onewire_config *ow_conf;
} mqtt_task_param_t;

static void mqtt_task(void *param){
    char *c_temp = new char[10];
    char *c_snq  = new char[10];
    char *c_top  = new char[50];
    onewire_device* ow_dev = NULL;

    mqtt_task_param_t* p = (mqtt_task_param_t*) param;

    while(1){
        vTaskDelay(pdMS_TO_TICKS(10000));
        for(int i = 0; i<p->ow_conf->get_num_devices(); i++){
            ow_dev = p->ow_conf->get_device(i);

            std::string sens_name = "";
            std::string sens_addr = ow_dev->addr_to_string();
            if(p->sett->key_exists(sens_addr)){
                sens_name = p->sett->get_string_value(sens_addr);
            }else{
                sens_name = sens_addr;
                p->sett->set_string_value(sens_addr, sens_addr);
                p->sett->save_to_file();
            }

            std::string ow_topic_fragment = p->sett->get_string_value("ow_topic_fragment");
            
            
            if(ow_dev->get_crc()){
                sprintf(c_temp, "%.2f", ow_dev->get_temperature());
                ESP_LOGI("mqtt_task", "%s: %s °C", sens_name.c_str(), c_temp);
                sprintf(c_top, "%s/%s", ow_topic_fragment.c_str(), sens_name.c_str());
                p->mqtt_cl->publish(c_top, c_temp, 0, false);
            }else{
                 ESP_LOGI("mqtt_task", "%s: failed", sens_name.c_str());
            }
            sprintf(c_snq, "%d", ow_dev->get_snq());
            sprintf(c_top, "%s/%s_snq", ow_topic_fragment.c_str(), sens_name.c_str());
            p->mqtt_cl->publish(c_top, c_snq, 0, false);
            ESP_LOGI("mqtt_task", "%s: %s (snq)", sens_name.c_str(), c_snq);
        }
    }
}

void setup_mqtt_logger(char *TAG, global_settings* sett){
    ESP_LOGI(TAG, "configuring MQTT-Client...");
    mqtt_client_config* mqtt_conf = new mqtt_client_config();
    mqtt_conf->set_host(sett->get_string_value("mqtt_server"));
    mqtt_conf->set_port(sett->get_int_value("mqtt_port"));

    ESP_LOGI(TAG, "starting MQTT-Client...");
    mqtt_client* mqtt_cl = new mqtt_client(mqtt_conf);

    ESP_LOGI(TAG, "connecting MQTT-Client...");
    mqtt_cl->connect();

    xEventGroupWaitBits(mqtt_conf->get_event_group(), MQTT_CONNECTED_BIT, false, false, portMAX_DELAY);

    ESP_LOGI(TAG, "starting OneWire-Client...");

    onewire_config *ow_conf = new onewire_config(GPIO_NUM_5);

    onewire_client *ow_client = new onewire_client(ow_conf);

    ESP_LOGI(TAG, "starting continous reading...");

    ow_client->continous_read();

    mqtt_task_param_t* p = new mqtt_task_param_t;
    p->sett = sett;
    p->mqtt_cl = mqtt_cl;
    p->ow_conf = ow_conf;
    
    TaskHandle_t run_task_handle = NULL;
    xTaskCreate(mqtt_task, "MQTT_TASK", 8196, p, tskIDLE_PRIORITY+2, &run_task_handle); 

}