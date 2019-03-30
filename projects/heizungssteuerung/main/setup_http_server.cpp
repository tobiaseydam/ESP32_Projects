#include "esp_log.h"
#include "tey_network.hpp"
#include "tey_http.hpp"
#include "tey_http_request_parser.hpp"
#include "tey_settings.hpp"
#include "tey_html_template_processor.hpp"

static std::string var_callback(std::string tag, void* ctx){
    ESP_LOGI("callback", "processing: %s", tag.c_str());

    global_settings* sett = (global_settings*)ctx;

    if(sett->key_exists(tag)){
        return sett->get_string_value(tag);
    }

    return "xxx";
}

void setup_http_server(char *TAG, global_settings* sett, storage_handle* stor_root, 
    storage_client* stor_cl){
    ESP_LOGI(TAG, "starting http-Server...");
    http_config* http_conf = new http_config();
    http_server* server = new http_server(http_conf);
    server->start();

    // edit_settings - event handler

    http_uri_handler* handler_edit = new http_uri_handler();
    handler_edit->set_uri("/edit_settings");
    handler_edit->set_method(HTTP_POST);

    handler_edit->set_handler([](httpd_req_t *req){
        ESP_LOGI("handler_edit", "-------------CONN-------------");
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
        
        std::string resp = "Settings updated <br><a href='/spiffs'>back</a>";
        httpd_resp_send(req, resp.c_str(), resp.length());

        return ESP_OK;
    });

    handler_edit->set_user_ctx(sett);

    // edit_settings - event handler

    http_uri_handler* handler_upload = new http_uri_handler();
    handler_upload->set_uri("/upload");
    handler_upload->set_method(HTTP_POST);

    handler_upload->set_handler([](httpd_req_t *req){
        ESP_LOGI("handler_upload", "-------------CONN-------------");
        http_request_parser p(req);

        p.set_upload_found_callback([](std::string field, std::string filename, std::string value, void* ctx){
            storage_handle* loc_stor_root = (storage_handle*) ctx;
            ESP_LOGI("upload_found_callback", "-------------UPLOAD------------");
            ESP_LOGI("upload_found_callback", "field: %s", field.c_str());
            ESP_LOGI("upload_found_callback", "filename: %s", filename.c_str());
            ESP_LOGI("upload_found_callback", "value: ...");

            storage_handle* c = loc_stor_root->create_child(filename);
            c->save_string(value);
            delete c;

            ESP_LOGI("field_found_callback", "------------------------------");
        });

        p.set_upload_found_ctx(req->user_ctx);
        p.parse();
        
        std::string resp = "File uploaded <br><a href='/spiffs'>back</a>";
        httpd_resp_send(req, resp.c_str(), resp.length());

        return ESP_OK;
    });

    handler_upload->set_user_ctx(stor_root);

    // config - event handler

    http_uri_handler* handler_config = new http_uri_handler();
    handler_config->set_uri("/spiffs");
    handler_config->set_method(HTTP_GET);

    handler_config->set_handler([](httpd_req_t *req){
        ESP_LOGI("handler_config", "-------------CONN-------------");
        storage_client* stor_cl = (storage_client*) req->user_ctx;
        storage_handle* stor_root = stor_cl->get_root();
        
        const uint8_t buf_len = httpd_req_get_url_query_len(req) + 1;
        char* buffer = new char[buf_len]; 
        char* resp = new char[127];

        std::string action = "";
        std::string filename = "";

        if (httpd_req_get_url_query_str(req, buffer, buf_len) == ESP_OK) {
                ESP_LOGI("heizungssteuerung", "Found URL query => %s", buffer);
                char param[32];
                if (httpd_query_key_value(buffer, "action", param, sizeof(param)) == ESP_OK) {
                    action = param;
                    ESP_LOGI("heizungssteuerung", "Found URL query parameter => action=%s", param);
                }
                if (httpd_query_key_value(buffer, "file", param, sizeof(param)) == ESP_OK) {
                    filename = param;
                    ESP_LOGI("heizungssteuerung", "Found URL query parameter => file=%s", param);
                }
        }

        if(action.compare("open")==0){
            storage_handle* file = new storage_handle(filename);
           if(file->exists()){
                std::string ext = file->get_extension();
                if((ext.compare(".htm") == 0) || (ext.compare(".html") == 0)){
                    httpd_resp_set_type(req, "text/html");
                    
                    storage_handle* settings_file = new storage_handle("/spiffs/config.json");
                    global_settings* sett = new global_settings(settings_file);
                    sett->load_from_file();
                    
                    template_processor proc = template_processor(file->get_content_as_string());
                    proc.set_process_var_callback(var_callback);
                    proc.set_process_ctx(sett);
                    proc.process();

                    httpd_resp_send_chunk(req, proc.get_output().c_str(), proc.get_output().length());
                    httpd_resp_send_chunk(req, NULL, 0);

                }else{
                    httpd_resp_set_type(req, "text");
                    sprintf(resp, "File: %s \n\n", filename.c_str());
                    httpd_resp_send_chunk(req, resp, strlen(resp));

                    FILE* f = file->open("r");
                    while(fgets(resp, 127, f)){
                        httpd_resp_send_chunk(req, resp, strlen(resp));
                    }

                    httpd_resp_send_chunk(req, NULL, 0);
                    file->close();
                }
            }else{
                sprintf(resp, "File not found: %s", filename.c_str());
                httpd_resp_send(req, resp, strlen(resp));
            }
            return ESP_OK;
        }

        if(action.compare("delete")==0){
            storage_handle* file = new storage_handle(filename);
            if(file->exists()){
                file->close();
                file->remove_file();
                sprintf(resp, "File deleted: %s<br><a href='/spiffs'>back</a>", filename.c_str());
                httpd_resp_send(req, resp, strlen(resp));
            }else{
                sprintf(resp, "File not found: %s", filename.c_str());
                httpd_resp_send(req, resp, strlen(resp));
            }
            return ESP_OK;
        }

        strcpy(resp, "Filesystem: <br>");
        httpd_resp_send_chunk(req, resp, strlen(resp));
        
        storage_handle* c = stor_root->get_first_child();
        
        sprintf(resp, "<table>");
        httpd_resp_send_chunk(req, resp, strlen(resp));

        while(c != NULL){

            sprintf(resp, "<tr><td>");
            httpd_resp_send_chunk(req, resp, strlen(resp));

            sprintf(resp, "<a href='?file=%s&action=open'>%s</a>", c->get_filename().c_str(), c->get_filename().c_str());
            httpd_resp_send_chunk(req, resp, strlen(resp));

            sprintf(resp, "</td><td>%ld</td><td>", c->get_size());
            httpd_resp_send_chunk(req, resp, strlen(resp));

            sprintf(resp, "<a href='?file=%s&action=delete'>delete</a>", c->get_filename().c_str());
            httpd_resp_send_chunk(req, resp, strlen(resp));

            sprintf(resp, "</td></tr>");
            httpd_resp_send_chunk(req, resp, strlen(resp));

            delete c;
            c = stor_root->get_next_child();
        }

        sprintf(resp, "</table>");
        httpd_resp_send_chunk(req, resp, strlen(resp));

        sprintf(resp, "<form action=\"upload\" method=\"post\" enctype=\"multipart/form-data\">");
        httpd_resp_send_chunk(req, resp, strlen(resp));

        sprintf(resp, "<p><input type=\"file\" name=\"uploadfile\"></p>");
        httpd_resp_send_chunk(req, resp, strlen(resp));

        sprintf(resp, "<p><button type=\"submit\">upload</button></p>");
        httpd_resp_send_chunk(req, resp, strlen(resp));
        
        sprintf(resp, "</form>");
        httpd_resp_send_chunk(req, resp, strlen(resp));

        httpd_resp_send_chunk(req, NULL, 0);
        free(resp);

        return ESP_OK;
    });
    

    handler_config->set_user_ctx(stor_cl);

    ESP_LOGI(TAG, "register uri-handler...");

    server->register_uri_handler(handler_edit);
    server->register_uri_handler(handler_upload);
    server->register_uri_handler(handler_config);

    ESP_LOGI(TAG, "http-Server online");
}