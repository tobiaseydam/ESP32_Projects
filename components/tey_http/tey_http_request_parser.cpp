#include "tey_http_request_parser.hpp"
#include "esp_err.h"
#include "esp_log.h"

http_request_parser::http_request_parser(httpd_req_t *request){
    req = request;
}

std::string http_request_parser::get_header(std::string block, std::string header){
    int p1 = block.find(header);
    int p2 = block.find("\n", p1 + header.length());
    if(p1 == -1){
        return "";
    }
    std::string value = block.substr(p1+header.length(), p2-p1-header.length());
    return value;
}

std::string http_request_parser::get_block_value(std::string block){
    int p1 = 0;
    int p2 = 0;
    while (1){
        p1 = block.find("\n", p1) + 1;
        p2 = block.find("\n", p1);
        if(p2 - p1 == 1)
            break;
        if(p2 == -1)
            break;
    }
    p1 = p2;
    std::string value = block.substr(p2+1, block.length()-p2);
    return value;
}

std::string http_request_parser::get_header_field(std::string header, std::string field){
    int p1 = header.find(field);
    int p2 = header.find("\"", p1 + field.length()+1);
    if(p1 == -1){   // Feld nicht gefunden
        return "";
    }
    if(p2 - p1 - field.length() == 1){  // Feld gefunden, aber leer
        return "\\";
    }
    std::string value = header.substr(p1+field.length()+1, p2-p1-field.length()-1);
    return value;
}

esp_err_t http_request_parser::parse_block(std::string block){    
    std::string content_disposition = get_header(block, "Content-Disposition: ");
    std::string name = get_header_field(content_disposition, "name=");
    std::string filename = get_header_field(content_disposition, "filename=");
    std::string content_type = get_header(block, "Content-Type: ");
    std::string value = get_block_value(block);

    if(!name.empty()){
        if(!filename.empty()){
            if(upload_field_found != NULL){
                upload_field_found(name, filename, value);
            }
        }else{
            if(field_found != NULL){
                field_found(name, value);
            }
        }
    }

    return ESP_OK;
}

esp_err_t http_request_parser::parse(){
    const uint8_t buffer_size = 128;
    char buffer[buffer_size];
    int remaining = req->content_len;
    uint8_t i = 0;
    esp_err_t ret;

    std::string line = "";
    std::string block_divider = "";
    std::string block = "";
    
    while (remaining > 0) {
        memset(&buffer, 0, sizeof(buffer));
        if ((ret = httpd_req_recv(req, buffer, MIN(remaining, sizeof(buffer)))) < 0) {
            return ESP_OK;
        }
        remaining -= ret;
        i = 0;
        while (i<buffer_size){
            if(buffer[i]=='\n'){
                if(block_divider == ""){
                    block_divider = line.substr(0,line.length()-1);
                }else if(block_divider.compare(line.substr(0,block_divider.length()))==0){
                    parse_block(block);
                    block = "";
                }else{
                    if(block == ""){
                        block = line;
                    }else{
                        block = block + "\n" + line;
                    }
                }
                line = "";
            }else{
                line += buffer[i];
            }
            i++;
        }
    }
    return ESP_OK;
}