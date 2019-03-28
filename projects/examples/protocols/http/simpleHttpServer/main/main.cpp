#include "esp_log.h"
#include "tey_network.hpp"
#include "tey_http.hpp"

extern "C" {
    void app_main(void);
}


void app_main(void){
    ESP_LOGI("simpleHttpServer", "configuring ETH-Connection...");
    eth_config* eth_conf = new eth_config();

    ESP_LOGI("simpleHttpServer", "establishing ETH-Connection...");
    network_adapter* nw = new network_adapter(eth_conf);

    nw->connect();
    xEventGroupWaitBits(eth_conf->get_event_group(), NW_GOT_IP_BIT, false, false, portMAX_DELAY);

    ESP_LOGI("simpleHttpServer", "starting http-Server...");
    http_config* http_conf = new http_config();
    http_server* server = new http_server(http_conf);
    server->start();

    ESP_LOGI("simpleHttpServer", "register uri-handler...");

    http_uri_handler* h = new http_uri_handler();
    h->set_uri("/test");
    h->set_method(HTTP_GET);
    h->set_handler([](httpd_req_t *req){
        int remaining = req->content_len;
        char buffer[256];
        esp_err_t ret;
        while (remaining > 0){
            if ((ret = httpd_req_recv(req, buffer, MIN(remaining, sizeof(buffer)))) < 0) {
                return ESP_FAIL;
            }
            remaining -= ret;
        }
        std::string resp = "Hello World";
        httpd_resp_send(req, resp.c_str(), resp.length());
        return ESP_OK;
    });
    h->set_user_ctx(NULL);
    server->register_uri_handler(h);

    ESP_LOGI("simpleHttpServer", "http-Server online");

}