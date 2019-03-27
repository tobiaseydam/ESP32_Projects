#include "esp_log.h"
#include "tey_network.hpp"
#include "tey_mqtt.hpp"
//#include "mqtt_client.h"

extern "C" {
    void app_main(void);
}

void app_main(void){
    ESP_LOGI("mqttMosquittoConnection", "configuring ETH-Connection...");
    eth_config* eth_conf = new eth_config();

    ESP_LOGI("mqttMosquittoConnection", "establishing ETH-Connection...");
    network_adapter* nw = new network_adapter(eth_conf);

    nw->connect();
      
    xEventGroupWaitBits(eth_conf->get_event_group(), NW_GOT_IP_BIT, false, false, portMAX_DELAY);

    ESP_LOGI("mqttMosquittoConnection", "ETH-Connection established");
    
    ESP_LOGI("mqttMosquittoConnection", "configuring MQTT-Client...");
    mqtt_client_config* mqtt_conf = new mqtt_client_config();
    mqtt_conf->set_host("192.168.178.23");
    mqtt_conf->set_port(1883);

    ESP_LOGI("mqttMosquittoConnection", "starting MQTT-Client...");
    mqtt_client* cl = new mqtt_client(mqtt_conf);

    ESP_LOGI("mqttMosquittoConnection", "connecting MQTT-Client...");
    cl->connect();

    xEventGroupWaitBits(mqtt_conf->get_event_group(), MQTT_CONNECTED_BIT, false, false, portMAX_DELAY);
    ESP_LOGI("mqttMosquittoConnection", "publishing Message...");
    int msg_id = cl->publish("ow/ESP32_test/onewire", "1", 0, false);

    if (msg_id >= 0) {
        ESP_LOGI("mqttMosquittoConnection", "Message (msg_id = %d) successfully published", msg_id);
    }
    
}