#include "tey_http.hpp"
#include "esp_log.h"
#include <string.h>


http_config::http_config(){
    server_handle = new httpd_handle_t;
}

http_server::http_server(http_config* config){
    cfg = config;
}

void http_server::start(){
    httpd_config_t config;
    memset(&config, 0, sizeof(config)); 
    config.task_priority      = tskIDLE_PRIORITY+5;
    config.stack_size         = 4096;
    config.server_port        = 80;
    config.ctrl_port          = 32768;
    config.max_open_sockets   = 7;
    config.max_uri_handlers   = 8;
    config.max_resp_headers   = 8;
    config.backlog_conn       = 5;
    config.lru_purge_enable   = false;
    config.recv_wait_timeout  = 5;
    config.send_wait_timeout  = 5;
    config.global_user_ctx    = NULL;
    config.global_user_ctx_free_fn = NULL;
    config.global_transport_ctx = NULL;
    config.global_transport_ctx_free_fn = NULL;
    config.open_fn            = NULL;
    config.close_fn           = NULL;

    httpd_handle_t* server_handle = cfg->get_server_handle();
    ESP_ERROR_CHECK(httpd_start(server_handle, &config));
}

void http_server::register_uri_handler(http_uri_handler* handler){
    httpd_uri_t u;
    memset(&u, 0, sizeof(u));
    u.uri = handler->get_uri().c_str();
    ESP_LOGI(TAG, "%s", handler->get_uri().c_str());
    u.method = handler->get_method();
    u.handler = handler->get_handler();
    u.user_ctx = handler->get_user_ctx();
    httpd_handle_t* server_handle = cfg->get_server_handle();
    ESP_ERROR_CHECK(httpd_register_uri_handler(*server_handle, &u));
}