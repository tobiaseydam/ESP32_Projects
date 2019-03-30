#include "esp_log.h"
#include "tey_network.hpp"
#include "tey_http.hpp"
#include "tey_http_request_parser.hpp"
#include "tey_settings.hpp"

extern "C" {
    void app_main(void);
}

void setup_http_server(char *TAG, global_settings* sett, storage_handle* stor_root, 
    storage_client* stor_cl);
void setup_mqtt_logger(char *TAG, global_settings* sett);

static constexpr char *TAG = (char*)"heizungssteuerung";

void print_spiffs(storage_handle* h){
    storage_handle* c = h->get_first_child();

    while(c != NULL){
        ESP_LOGI("SPIFFS", "%-32s - %5ld Bytes - %-5s - %-4s", 
            c->get_filename().c_str(), c->get_size(), 
            c->get_extension().c_str(), c->is_file()?"FILE":"DIR");
        delete c;
        c = h->get_next_child();
    }
}

typedef struct{
    void* p1;
    void* p2;
}httpConfigTool_ctx_t;

void app_main(void){
    ESP_LOGI(TAG, "configuring ETH-Connection...");
    eth_config* eth_conf = new eth_config();

    ESP_LOGI(TAG, "establishing ETH-Connection...");
    network_adapter* nw = new network_adapter(eth_conf);

    storage_config* stor_conf = new storage_config(SPIFFS, "/spiffs");
    storage_client* stor_cl = new storage_client(stor_conf);
    storage_handle* stor_root = stor_cl->get_root();

    ESP_LOGI(TAG, "stor_root = %p", stor_root);

    print_spiffs(stor_root);

    storage_handle* settings_file = new storage_handle("/spiffs/config.json");
    global_settings* sett = new global_settings(settings_file);
    sett->load_from_file();

    if(!sett->key_exists("ow_topic_fragment")){
        sett->set_string_value("ow_topic_fragment", "ESP32_Heizungssteuerung"); 
    }
    if(!sett->key_exists("mqtt_server")){
        sett->set_string_value("mqtt_server", "192.168.178.23"); 
    }
    if(!sett->key_exists("mqtt_port")){
        sett->set_int_value("mqtt_port", 1883); 
    }
    sett->save_to_file();

    nw->connect();
    xEventGroupWaitBits(eth_conf->get_event_group(), NW_GOT_IP_BIT, false, false, portMAX_DELAY);

    setup_http_server(TAG, sett, stor_root, stor_cl);
    setup_mqtt_logger(TAG, sett);
}