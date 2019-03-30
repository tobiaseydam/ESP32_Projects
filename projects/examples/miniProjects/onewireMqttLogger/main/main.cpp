#include "esp_log.h"
#include "tey_network.hpp"
#include "tey_mqtt.hpp"
#include "tey_onewire.hpp"

extern "C" {
    void app_main(void);
}

void app_main(void){
    ESP_LOGI("onewireMqttLogger", "configuring ETH-Connection...");
    eth_config* eth_conf = new eth_config();

    ESP_LOGI("onewireMqttLogger", "establishing ETH-Connection...");
    network_adapter* nw = new network_adapter(eth_conf);

    nw->connect();
      
    xEventGroupWaitBits(eth_conf->get_event_group(), NW_GOT_IP_BIT, false, false, portMAX_DELAY);

    ESP_LOGI("onewireMqttLogger", "ETH-Connection established");
    
    ESP_LOGI("onewireMqttLogger", "configuring MQTT-Client...");
    mqtt_client_config* mqtt_conf = new mqtt_client_config();
    mqtt_conf->set_host("192.168.178.23");
    mqtt_conf->set_port(1883);

    ESP_LOGI("onewireMqttLogger", "starting MQTT-Client...");
    mqtt_client* mqtt_cl = new mqtt_client(mqtt_conf);

    ESP_LOGI("onewireMqttLogger", "connecting MQTT-Client...");
    mqtt_cl->connect();

    xEventGroupWaitBits(mqtt_conf->get_event_group(), MQTT_CONNECTED_BIT, false, false, portMAX_DELAY);

    ESP_LOGI("onewireMqttLogger", "starting OneWire-Client...");

    onewire_config *ow_conf = new onewire_config(GPIO_NUM_5);

    onewire_client *ow_client = new onewire_client(ow_conf);

    ESP_LOGI("onewireMqttLogger", "starting continous reading...");

    ow_client->continous_read();

    char *c_temp = new char[10];
    char *c_top  = new char[50];
    onewire_device* ow_dev = NULL;

    while(1){
        vTaskDelay(pdMS_TO_TICKS(10000));
        for(int i = 0; i<ow_conf->get_num_devices(); i++){
             ow_dev = ow_conf->get_device(i);
             if(ow_dev->get_crc()){
                sprintf(c_temp, "%.2f", ow_dev->get_temperature());
                ESP_LOGI("onewireMqttLogger", "Temperatur %d: %s °C", i + 1, c_temp);
                sprintf(c_top, "ow/ESP32_test/TempSensor%d", i + 1);
                mqtt_cl->publish(c_top, c_temp, 0, false);
             }else{
                 ESP_LOGI("onewireMqttLogger", "Sensor %d: failed", i + 1);
             }
        }
    }

}