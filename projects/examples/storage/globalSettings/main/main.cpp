#include "esp_log.h"
#include "tey_settings.hpp"


extern "C" {
    void app_main(void);
}

void app_main(void){

    storage_config* conf = new storage_config(SPIFFS, "/spiffs");
    storage_client* cl = new storage_client(conf);

    storage_handle* h = cl->get_root();
    storage_handle* settings_file = h->create_child("settings.json");

    global_settings* sett_save = new global_settings(settings_file);
    
    ESP_LOGI("globalSettings", "create new settings file");
    sett_save->set_string_value("test_string", "asd");
    sett_save->set_int_value("test_int", 1);
    sett_save->set_bool_value("test_bool1", true);
    sett_save->set_bool_value("test_bool2", false);
    sett_save->set_double_value("test_double", 50.3);

    ESP_LOGI("globalSettings", "save settings file");
    sett_save->save_to_file();

    global_settings* sett_load = new global_settings(settings_file);


    ESP_LOGI("globalSettings", "load settings from file");
    sett_load->load_from_file();

    ESP_LOGI("globalSettings", "output settings");

    ESP_LOGI("globalSettings", "test_string: %s", 
        sett_load->get_string_value("test_string").c_str());
    ESP_LOGI("globalSettings", "test_int: %d", 
        sett_load->get_int_value("test_int"));
    ESP_LOGI("globalSettings", "test_bool1: %d", 
        sett_load->get_bool_value("test_bool1"));
    ESP_LOGI("globalSettings", "test_bool2: %d", 
        sett_load->get_bool_value("test_bool2"));
    ESP_LOGI("globalSettings", "test_double: %.2f", 
        sett_load->get_double_value("test_double"));

}