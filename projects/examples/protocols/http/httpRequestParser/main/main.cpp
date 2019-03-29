#include "esp_log.h"
#include "tey_network.hpp"
#include "tey_http.hpp"
#include "tey_http_request_parser.hpp"

extern "C" {
    void app_main(void);
}

static void field_found_callback(std::string field, std::string value){
    ESP_LOGI("field_found_callback", "-------------FIELD------------");
    ESP_LOGI("field_found_callback", "field: %s", field.c_str());
    ESP_LOGI("field_found_callback", "value: %s", value.c_str());
    ESP_LOGI("field_found_callback", "------------------------------");
}

static void upload_found_callback(std::string field, std::string filename, std::string value){
    ESP_LOGI("field_found_callback", "-------------UPLOAD-----------");
    ESP_LOGI("field_found_callback", "field:    %s", field.c_str());
    ESP_LOGI("field_found_callback", "filename: %s", filename.c_str());
    ESP_LOGI("field_found_callback", "value:    %s", value.c_str());
    ESP_LOGI("field_found_callback", "------------------------------");
}

void app_main(void){
    ESP_LOGI("httpRequestParser", "configuring ETH-Connection...");
    eth_config* eth_conf = new eth_config();

    ESP_LOGI("httpRequestParser", "establishing ETH-Connection...");
    network_adapter* nw = new network_adapter(eth_conf);

    nw->connect();
    xEventGroupWaitBits(eth_conf->get_event_group(), NW_GOT_IP_BIT, false, false, portMAX_DELAY);

    ESP_LOGI("httpRequestParser", "starting http-Server...");
    http_config* http_conf = new http_config();
    http_server* server = new http_server(http_conf);
    server->start();

    ESP_LOGI("httpRequestParser", "register uri-handler...");

    http_uri_handler* h = new http_uri_handler();
    h->set_uri("/test");
    h->set_method(HTTP_POST);

    h->set_handler([](httpd_req_t *req){
        http_request_parser p(req);
        p.set_field_found_callback(field_found_callback);
        p.set_upload_found_callback(upload_found_callback);
        p.parse();

        std::string resp = "Hello World";
        httpd_resp_send(req, resp.c_str(), resp.length());

        return ESP_OK;
    });

    h->set_user_ctx(NULL);
    server->register_uri_handler(h);

    ESP_LOGI("httpRequestParser", "http-Server online");

}