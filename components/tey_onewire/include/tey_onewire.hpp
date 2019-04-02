#ifndef TEY_ONEWIRE_HPP
    #define TEY_ONEWIRE_HPP

#include "esp_err.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string>

#define MAX_NUM_ONEWIRE_DEVICES                 8
#define SNQ_LEN_ONEWIRE_DEVICES                10
#define MAX_READ_RETRY_ONEWIRE_DEVICES          5

typedef struct onewire_addr { uint8_t x[8]; } onewire_addr_t;
typedef struct onewire_data { uint8_t x[9]; } onewire_data_t;

class onewire_device{
    private:
        static constexpr char *TAG = (char*)"onewire_device";
    protected:
        onewire_addr_t addr;
        onewire_data_t data;
        bool b_crc;
        uint8_t hlp_crc(uint8_t crc, uint8_t d);
        uint16_t readings = 0;
        uint16_t fails = 0;
        uint8_t snq_len = SNQ_LEN_ONEWIRE_DEVICES;
        bool snq_arr[SNQ_LEN_ONEWIRE_DEVICES];
        uint8_t snq;
        void print_snq();
    public:
        onewire_device(onewire_addr_t a);
        onewire_addr_t get_addr() { return addr; };
        onewire_data_t get_data() { return data; };
        bool get_crc() { return b_crc; };
        void set_data(onewire_data_t d);
        double get_temperature();
        void check_crc();
        uint16_t get_readings() { return readings; };
        uint16_t get_fails() { return fails; };
        uint8_t get_snq() { return snq; };
        std::string addr_to_string();
        std::string data_to_string();
};

class onewire_config{
    private:
        static constexpr char *TAG = (char*)"onewire_config";
    protected:
        gpio_num_t pin = GPIO_NUM_15;
        uint8_t read_intervall = 10;
        uint8_t num_devices = 0;
        onewire_device* devices[MAX_NUM_ONEWIRE_DEVICES];
    public:
        onewire_config(gpio_num_t hw_pin);

        gpio_num_t  get_pin() { return pin; }
        
        uint8_t get_read_interval() { return read_intervall; };
        void set_read_interval(uint8_t value) { read_intervall = value; };

        int get_num_devices() { return num_devices; };

        onewire_device* get_device(uint8_t idx);
        void add_device(onewire_device* d);
};

class onewire_client{
    private:
        static constexpr char *TAG = (char*)"onewire_client";
    protected:
        static uint8_t reset_pulse();

        static void send_bit(uint8_t data);
        static uint8_t read_bit();
        
        static void send_byte(uint8_t data);
       static  uint8_t read_byte();

        static bool check_mask(onewire_addr_t mask, uint8_t bit, uint8_t byte);
        static void search_devices();

        static void read_data();
        static void run_task(void *param);

        static onewire_config *conf;
        TaskHandle_t run_task_handle = NULL;
    public:
        onewire_client(onewire_config *ow_conf);
        void continous_read();
};

#endif