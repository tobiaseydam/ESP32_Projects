#include "esp_log.h"
#include "tey_network.hpp"
#include "tey_http.hpp"
#include "tey_http_request_parser.hpp"
#include "tey_settings.hpp"

extern "C" {
    void app_main(void);
}

static constexpr char *TAG = (char*)"httpSettingsEditor";

void app_main(void){
    ESP_LOGI(TAG, "configuring ETH-Connection...");
    eth_config* eth_conf = new eth_config();

    ESP_LOGI(TAG, "establishing ETH-Connection...");
    network_adapter* nw = new network_adapter(eth_conf);

    storage_config* stor_conf = new storage_config(SPIFFS, "/spiffs");
    storage_client* stor_cl = new storage_client(stor_conf);
    storage_handle* stor_root = stor_cl->get_root();

    storage_handle* settings_file = new storage_handle("/spiffs/settings2.json");
    global_settings* sett = new global_settings(settings_file);
    sett->load_from_file();

    nw->connect();
    xEventGroupWaitBits(eth_conf->get_event_group(), NW_GOT_IP_BIT, false, false, portMAX_DELAY);

    ESP_LOGI(TAG, "starting http-Server...");
    http_config* http_conf = new http_config();
    http_server* server = new http_server(http_conf);
    server->start();

    http_uri_handler* handler_edit = new http_uri_handler();
    handler_edit->set_uri("/edit_settings");
    handler_edit->set_method(HTTP_POST);

    handler_edit->set_handler([](httpd_req_t *req){
        ESP_LOGI("field_found_callback", "-------------CONN-------------");
        http_request_parser p(req);
        p.set_field_found_callback([](std::string field, std::string value, void* ctx){
            global_settings* loc_sett = (global_settings*) ctx;
            ESP_LOGI("field_found_callback", "-------------FIELD------------");
            ESP_LOGI("field_found_callback", "field: %s", field.c_str());
            ESP_LOGI("field_found_callback", "value: %s", value.c_str());

            loc_sett->set_string_value(field, value);

            ESP_LOGI("field_found_callback", "------------------------------");
        });
        p.set_field_found_ctx(req->user_ctx);
        p.parse();

        global_settings* loc_sett = (global_settings*) req->user_ctx;
        loc_sett->save_to_file();
        
        std::string resp = "done";
        httpd_resp_send(req, resp.c_str(), resp.length());

        return ESP_OK;
    });

    handler_edit->set_user_ctx(sett);

    ESP_LOGI(TAG, "register uri-handler...");
    server->register_uri_handler(handler_edit);

    ESP_LOGI(TAG, "http-Server online");

}