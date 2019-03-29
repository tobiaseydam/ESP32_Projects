#include "tey_settings.hpp"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>

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
    
    for (auto const& elem : int_map){
        cJSON *cj_elem = cJSON_CreateObject();

        cJSON *cj_elem_type = cJSON_CreateString("int");
        cJSON_AddItemToObject(cj_elem, "type",  cj_elem_type);

        cJSON *cj_elem_key = cJSON_CreateString(elem.first.c_str());
        cJSON_AddItemToObject(cj_elem, "key",  cj_elem_key);

        cJSON *cj_elem_value = cJSON_CreateNumber(elem.second);
        cJSON_AddItemToObject(cj_elem, "value",  cj_elem_value);

        cJSON_AddItemToArray(cj_settings, cj_elem);
    }
    
    for (auto const& elem : bool_map){
        cJSON *cj_elem = cJSON_CreateObject();

        cJSON *cj_elem_type = cJSON_CreateString("bool");
        cJSON_AddItemToObject(cj_elem, "type",  cj_elem_type);

        cJSON *cj_elem_key = cJSON_CreateString(elem.first.c_str());
        cJSON_AddItemToObject(cj_elem, "key",  cj_elem_key);

        cJSON *cj_elem_value = cJSON_CreateBool(elem.second);
        cJSON_AddItemToObject(cj_elem, "value",  cj_elem_value);

        cJSON_AddItemToArray(cj_settings, cj_elem);
    }
    
    for (auto const& elem : double_map){
        cJSON *cj_elem = cJSON_CreateObject();

        cJSON *cj_elem_type = cJSON_CreateString("double");
        cJSON_AddItemToObject(cj_elem, "type",  cj_elem_type);

        cJSON *cj_elem_key = cJSON_CreateString(elem.first.c_str());
        cJSON_AddItemToObject(cj_elem, "key",  cj_elem_key);

        cJSON *cj_elem_value = cJSON_CreateNumber(elem.second);
        cJSON_AddItemToObject(cj_elem, "value",  cj_elem_value);

        cJSON_AddItemToArray(cj_settings, cj_elem);
    }

    std::string json = cJSON_Print(cj_settings);
    file->save_string(json);
}

void global_settings::load_from_file(){
    std::string file_content = file->get_content_as_string();
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
        if(strcmp(type->valuestring, "int")==0){
            int_map[s_key] = value->valueint;
        }
        if(strcmp(type->valuestring, "bool")==0){
            bool_map[s_key] = value->valueint == 1;
        }
        if(strcmp(type->valuestring, "double")==0){
            double_map[s_key] = value->valuedouble;
        }
    }
    
    cJSON_Delete(cj_settings);
}