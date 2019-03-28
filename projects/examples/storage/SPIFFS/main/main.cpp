#include "esp_log.h"
#include "tey_storage.hpp"

extern "C" {
    void app_main(void);
}

void print_spiffs(storage_handle* h){
    storage_handle* c = h->get_first_child();

    while(c != NULL){
        ESP_LOGI("SPIFFS", "%-32s - %5ld Bytes - %-5s - %-4s", 
            c->get_filename().c_str(), c->get_size(), 
            c->get_extension().c_str(), c->is_file()?"FILE":"DIR");
        delete c;
        c = h->get_next_child();
    }
}

void app_main(void){
    storage_config* conf = new storage_config(SPIFFS, "/spiffs");
    storage_client* cl = new storage_client(conf);

    storage_handle* h = cl->get_root();

    storage_handle* h1 = new storage_handle("/spiffs/testfile.txt");
    h1->remove_file();
    delete h1;

    storage_handle* h2 = new storage_handle("/spiffs/new_testfile.txt");
    h2->remove_file();
    delete h2;

    ESP_LOGI("SPIFFS", "Filesystem: ");
    print_spiffs(h);

    ESP_LOGI("SPIFFS", "Create new file...");
    storage_handle* tf = h->create_child("testfile.txt");
    
    ESP_LOGI("SPIFFS", "Save content to file...");
    tf->save_string("Testcontent");
    
    ESP_LOGI("SPIFFS", "Filesystem: ");
    print_spiffs(h);

    ESP_LOGI("SPIFFS", "Rename file...");
    tf->rename_file("/spiffs/new_testfile.txt");
    
    ESP_LOGI("SPIFFS", "Filesystem: ");
    print_spiffs(h);

    ESP_LOGI("SPIFFS", "Open file...");
    std::string tf_content = tf->get_content_as_string();
    ESP_LOGI("SPIFFS", "\n%s\n", tf_content.c_str());

    ESP_LOGI("SPIFFS", "Remove file...");
    tf->remove_file();
    delete tf;
    
    ESP_LOGI("SPIFFS", "Filesystem: ");
    print_spiffs(h);

    // direct open:
    //storage_handle* f = new storage_handle("/spiffs/testfile.txt");
    //std::string f_content = f->get_content_as_string();
}