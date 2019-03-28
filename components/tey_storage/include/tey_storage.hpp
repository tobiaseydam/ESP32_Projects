#ifndef TEY_STORAGE_HPP
    #define TEY_STORAGE_HPP

#include <string>
#include <dirent.h>

typedef enum{
    SPIFFS,
    SD
} storage_place_t;

class storage_handle{
    private:
        static constexpr char *TAG = (char*)"storage_handle";
    protected:
        FILE* handle = NULL;
        std::string name = "";
        int children_count = 0;
        DIR* dir_handle = NULL;
    public:
        storage_handle(std::string filename);
        bool exists();
        bool is_file();
        bool is_folder();
        std::string get_filename() { return name; };
        FILE* get_handle() { return handle; };
        long get_size();
        std::string get_extension();
        FILE* open(const char *mode);
        void close();
        int get_children_count(bool count);
        storage_handle* get_first_child();
        storage_handle* get_next_child();
        std::string get_content_as_string();
        void save_string(std::string content);
        void remove_file();
        void rename_file(std::string new_name);
        storage_handle* create_child(std::string filename);
};

class storage_config{
    private:
        static constexpr char *TAG = (char*)"storage_config";
    protected:
        storage_place_t src = SPIFFS;
        std::string root = "/spiffs";
    public:
        storage_config(storage_place_t stor_src, std::string root_path);
        storage_place_t get_storage_place() { return src; };
        std::string get_root_path() { return root; };
};

class storage_client{
    private:
        static constexpr char *TAG = (char*)"storage_client";
    protected:
        storage_config* cfg;
    public:
        storage_client(storage_config* config);
        storage_handle* get_root(); 
};

#endif