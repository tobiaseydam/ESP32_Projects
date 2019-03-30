#include "tey_html_template_processor.hpp"
#include "esp_log.h"

std::string trim(std::string str){
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last-first+1));
}

template_processor::template_processor(std::string templ){
    input = templ;
}

void template_processor::process_next_tag(std::string tag){
    ESP_LOGI(TAG, "processing: %s", tag.c_str());
    
    int p1 = tag.find("var:");
    if(p1 >= 0){
        if(var_cb != NULL){
            output = output + var_cb(trim(tag.substr(p1 + 4)), process_ctx);
        }
    }
}

void template_processor::process(){
    int p1, p2;
    std::string tag;
    while(pos < input.length()){
        p1 = input.find("<#", pos);
        output = output + input.substr(pos, p1 - pos);
        if(p1 >= 0){
            p2 = input.find("#>", p1);
            if(p2 >= 0){
                tag = input.substr(p1 + 2, p2 - p1 - 2);
                process_next_tag(trim(tag));
                pos = p2 + 2;
            }else{
                ESP_LOGI(TAG, "missing #>");
                break;
            }
        }else{
            ESP_LOGI(TAG, "done");
            break;
        }
    }
    //output = output + input.substr(pos + 2);
}