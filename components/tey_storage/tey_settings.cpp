#include "tey_settings.hpp"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


global_settings::global_settings(storage_handle* stor_file){
    file = stor_file;
}

void global_settings::save_to_file(){
    cJSON *cj_settings = cJSON_CreateArray();
    
    for (auto const& elem : str_map){
        cJSON *cj_elem = cJSON_CreateObject();

        cJSON *cj_elem_type = cJSON_CreateString("string");
        cJSON_AddItemToObject(cj_elem, "type",  cj_elem_type);

        cJSON *cj_elem_key = cJSON_CreateString(elem.first.c_str());
        cJSON_AddItemToObject(cj_elem, "key",  cj_elem_key);

        cJSON *cj_elem_value = cJSON_CreateString(elem.second.c_str());
        cJSON_AddItemToObject(cj_elem, "value",  cj_elem_value);

        cJSON_AddItemToArray(cj_settings, cj_elem);
    }

    std::string json = cJSON_Print(cj_settings);
    file->save_string(json);
}

void global_settings::load_from_file(){
    std::string file_content = file->get_content_as_string();
    
    if(file_content.empty()){
        return;
    }

    cJSON *cj_settings = cJSON_Parse(file_content.c_str());
    
    const cJSON *element = NULL;
    cJSON_ArrayForEach(element, cj_settings){
        
        cJSON *type  = cJSON_GetObjectItemCaseSensitive(element, "type");
        cJSON *key   = cJSON_GetObjectItemCaseSensitive(element, "key");
        cJSON *value = cJSON_GetObjectItemCaseSensitive(element, "value");

        std::string s_key = std::string(key->valuestring);

        if(strcmp(type->valuestring, "string")==0){
            std::string s_value = std::string(value->valuestring);
            str_map[s_key] = s_value;
        }
    }
    
    cJSON_Delete(cj_settings);
}

int global_settings::get_int_value(std::string key){
    return atoi(str_map[key].c_str());
}

void global_settings::set_int_value(std::string key, int value){
    char buffer [20];
    str_map[key] = itoa(value, buffer, 10);
}

bool global_settings::get_bool_value(std::string key){
    return atoi(str_map[key].c_str()) == 1;
}

void global_settings::set_bool_value(std::string key, bool value){
    str_map[key] = value?"1":"0";
}

double global_settings::get_double_value(std::string key){
    return atof(str_map[key].c_str());
}

void global_settings::set_double_value(std::string key, double value){
    char buffer [20];
    sprintf(buffer, "%f", value);
    str_map[key] = buffer;
}