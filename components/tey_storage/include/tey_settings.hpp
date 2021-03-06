#ifndef TEY_SETTINGS_HPP
    #define TEY_SETTINGS_HPP

#include "tey_storage.hpp"
#include <map>

class global_settings{
    private:
        static constexpr char *TAG = (char*)"global_settings";
    protected:
        storage_handle* file;
        std::map<std::string, std::string> str_map;
    public:
        global_settings(storage_handle* stor_file);
        void save_to_file();
        void load_from_file();

        std::string get_string_value(std::string key) 
            { return str_map[key]; };
        void set_string_value(std::string key, std::string value) 
            { str_map[key] = value; };

        int get_int_value(std::string key);
        void set_int_value(std::string key, int value);

        bool get_bool_value(std::string key);
        void set_bool_value(std::string key, bool value);

        double get_double_value(std::string key);
        void set_double_value(std::string key, double value);

        bool key_exists(std::string key)
            { return str_map.find(key) != str_map.end(); };
};

#endif