#include "tey_storage.hpp"

#include "esp_log.h"
#include "esp_spiffs.h"
#include <sys/stat.h>

storage_handle::storage_handle(std::string filename){
    name = filename;
};

bool storage_handle::exists(){
    struct stat buffer;   
    return (stat (name.c_str(), &buffer) == 0); 
}

bool storage_handle::is_file(){
    struct stat buffer;   
    stat (name.c_str(), &buffer);
    return buffer.st_mode & S_IFREG;
}

bool storage_handle::is_folder(){
    struct stat buffer;   
    stat (name.c_str(), &buffer);
    return buffer.st_mode & S_IFDIR;
}

long storage_handle::get_size(){
    struct stat buffer;   
    stat (name.c_str(), &buffer);
    return buffer.st_size;
}

std::string storage_handle::get_extension(){
    std::string ext = name.substr(name.find_last_of("."));
    return ext;
}

FILE* storage_handle::open(const char *mode){
    if(handle != NULL){
        close();
    }
    handle = fopen(name.c_str(), mode);
    return handle;
}

void storage_handle::close(){
    if(handle != NULL){
        fclose(handle);
    }
}

int storage_handle::get_children_count(bool count){
    if(is_file()){
        return 0;
    }

    if(count){
        children_count = 0;
        DIR *dir = opendir(name.c_str());
        while(readdir(dir)){
            children_count++;
        }
        closedir(dir);
    }

    return children_count;
}

storage_handle* storage_handle::get_first_child(){
    if(is_file()){
        return NULL;
    }
    if(dir_handle != NULL){
        closedir(dir_handle);
    }
    dir_handle = opendir(name.c_str());
    if(dir_handle == NULL){
        return NULL;
    }else{
        struct dirent *ent = readdir(dir_handle);
        if(ent == NULL){
            return NULL;
        }
        return new storage_handle(name + "/" + ent->d_name);
    }
}

storage_handle* storage_handle::get_next_child(){
    struct dirent *ent = readdir(dir_handle);
    if(ent == NULL){
        return NULL;
    }
    return new storage_handle(name + "/" + ent->d_name);
}

std::string storage_handle::get_content_as_string(){
    std::string content = "";
    char* buf = new char[127];
    if(exists() & is_file()){
        open("r");
        while(fgets(buf, 127, handle)){
            content = content + buf;
        }
        close();
    }
    delete buf;
    return content;
}

void storage_handle::save_string(std::string content){
    open("w");
    fprintf(handle, content.c_str());
    close();
}

void storage_handle::remove_file(){
    close();
    remove(name.c_str());
}

void storage_handle::rename_file(std::string new_name){
    close();
    rename(name.c_str(), new_name.c_str());
    name = new_name;
}

storage_handle* storage_handle::create_child(std::string filename){
    return new storage_handle(name + "/" + filename);
}

storage_config::storage_config(storage_place_t stor_src, std::string root_path){
    src = stor_src;
    root = root_path;
}

storage_client::storage_client(storage_config* config){
    cfg = config;

    if(cfg->get_storage_place() == SPIFFS){
        esp_vfs_spiffs_conf_t conf;
        conf.base_path = cfg->get_root_path().c_str();
        conf.partition_label = NULL;
        conf.max_files = 5;
        conf.format_if_mount_failed = true;
        
        esp_err_t ret = esp_vfs_spiffs_register(&conf);
        
        if (ret != ESP_OK) {
            if (ret == ESP_FAIL) {
                ESP_LOGE(TAG, "Failed to mount or format filesystem");
            } else if (ret == ESP_ERR_NOT_FOUND) {
                ESP_LOGE(TAG, "Failed to find SPIFFS partition");
            } else {
                ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
            }
            return;
        }

        size_t total = 0, used = 0;
        ret = esp_spiffs_info(NULL, &total, &used);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        } else {
            ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
        }
    }

}

storage_handle* storage_client::get_root(){
    return new storage_handle(cfg->get_root_path());
}