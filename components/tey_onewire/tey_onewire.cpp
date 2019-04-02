#include "tey_onewire.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "rom/ets_sys.h"

uint8_t onewire_device::hlp_crc(uint8_t crc, uint8_t d){
    uint8_t poly = 0x8c;
    for (uint8_t i = 8; i>0; i--){
        uint8_t mix = (crc^d) & 0x01;
        crc >>= 1;
        if(mix) crc ^= poly;
        d >>=1;
    }
    return crc;
}


void onewire_device::print_snq(){
    ESP_LOGI("TAG", "%d - %d - %d - %d - %d - %d - %d - %d - %d - %d", snq_arr[0], snq_arr[1], snq_arr[2], snq_arr[3], snq_arr[4], snq_arr[5], snq_arr[6], snq_arr[7], snq_arr[8], snq_arr[9]);
}

onewire_device::onewire_device(onewire_addr_t a){
    addr = a;
    ESP_LOGI(TAG, "new device: %02x %02x %02x %02x %02x %02x %02x %02x", a.x[0],a.x[1],a.x[2],a.x[3],a.x[4],a.x[5],a.x[6],a.x[7]);
    for(int i = 0; i<SNQ_LEN_ONEWIRE_DEVICES; i++){
        snq_arr[i] = false;
    }
    print_snq();
}


void onewire_device::set_data(onewire_data_t d){
    data = d;
    check_crc();
    readings++;
    if(!b_crc){
        fails++;
    }

    for(int i = 0; i<SNQ_LEN_ONEWIRE_DEVICES-1; i++){
        snq_arr[i] = snq_arr[i+1];
    }
    snq_arr[SNQ_LEN_ONEWIRE_DEVICES-1] = b_crc;
    print_snq();

    snq = 0;
    for(int i = 0; i<SNQ_LEN_ONEWIRE_DEVICES; i++){
        if(snq_arr[i]){
            snq++;
        }
    }

    if(readings<SNQ_LEN_ONEWIRE_DEVICES){
        snq = -1;
    }
}

double onewire_device::get_temperature(){
    uint8_t msb = data.x[1];
    uint8_t lsb = data.x[0];

    uint16_t raw = (msb<<8) + lsb;
    bool sign = (raw>>11>0);

    if(sign){
        return -((raw ^ 0xFFFF) + 1)/16.0;
    }else{
        return raw/16.0;
    }
}

void onewire_device::check_crc(){
    uint8_t crc = 0;
    for(uint8_t i = 0; i<9; i++){
        crc = hlp_crc(crc, data.x[i]);
    }
    b_crc = (crc==0);
}

std::string onewire_device::addr_to_string(){
    char buffer[24];
    sprintf(buffer, "%02x %02x %02x %02x %02x %02x %02x %02x", addr.x[0],addr.x[1],addr.x[2],addr.x[3],addr.x[4],addr.x[5],addr.x[6],addr.x[7]);
    return std::string(buffer);
}

std::string onewire_device::data_to_string(){
    char buffer[27];
    sprintf(buffer, "%02x %02x %02x %02x %02x %02x %02x %02x %02x", data.x[0],data.x[1],data.x[2],data.x[3],data.x[4],data.x[5],data.x[6],data.x[7],data.x[8]);
    return std::string(buffer);
}



onewire_config::onewire_config(gpio_num_t hw_pin){
    pin = hw_pin;
}

onewire_device* onewire_config::get_device(uint8_t idx){
    if(idx<num_devices){
        return devices[idx];
    }
    return NULL;
}

void onewire_config::add_device(onewire_device* d){
    devices[num_devices] = d;
    num_devices++;
}

onewire_config *onewire_client::conf = NULL;

uint8_t onewire_client::reset_pulse(){
    uint8_t PRESENCE;
    gpio_num_t pin = conf->get_pin();
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin,0);
    ets_delay_us(500);
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    ets_delay_us(30);
    if(gpio_get_level(pin)==0) PRESENCE=1; else PRESENCE=0;
    ets_delay_us(470);
    if(gpio_get_level(pin)==1) PRESENCE=1; else PRESENCE=0;
    return PRESENCE;
}

void onewire_client::send_bit(uint8_t data){
    gpio_num_t pin = conf->get_pin();
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin,0);
    ets_delay_us(5);
    if(data==1)gpio_set_direction(pin, GPIO_MODE_INPUT);
    ets_delay_us(80);
    gpio_set_direction(pin, GPIO_MODE_INPUT);
}

uint8_t onewire_client::read_bit(){
    uint8_t PRESENCE=0;
    gpio_num_t pin = conf->get_pin();
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin,0);
    ets_delay_us(2);
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    ets_delay_us(15);
    if(gpio_get_level(pin)==1) PRESENCE=1; else PRESENCE=0;
    return(PRESENCE);
}

void onewire_client::send_byte(uint8_t data){
    uint8_t i;
    uint8_t x;
    for(i=0;i<8;i++){
        x = data>>i;
        x &= 0x01;
        send_bit(x);
    }
    ets_delay_us(100);
}

uint8_t onewire_client::read_byte(){
    uint8_t i;
    uint8_t data = 0;
    for (i=0;i<8;i++){
        if(read_bit()) data|=0x01<<i;
        ets_delay_us(15);
    }
    return(data);
}

bool onewire_client::check_mask(onewire_addr_t mask, uint8_t bit, uint8_t byte){
    
    for(int i = 7; i>byte; i--){
        if(mask.x[i]>0){
            return false;
        }
    }

    uint8_t a = mask.x[byte]<<(bit+1);
    if(a>0){
        return false;
    }
    
    uint8_t b = mask.x[byte]<<(bit);
    if(b>0){
        return true;
    }

    return false;
}

void onewire_client::search_devices(){
    ESP_LOGI(TAG, "searching onewire devices");
    uint8_t b1=0, b2=0, b=0, m=0, s=0;
    int i, j, k = 0;
    bool first = true;
    bool finished = false;
    onewire_addr_t addr, mask;
    while(!finished){
        finished = true;
        if(reset_pulse() == 1){
            ESP_LOGI(TAG, "presence ok");
            // search rom
            send_byte(0xF0);
            for(j = 0; j<8; j++){
                for(i = 0; i<8; i++){
                    b1 = read_bit();
                    ets_delay_us(15);
                    b2 = read_bit();
                    ets_delay_us(15);
                    if(first || !check_mask(mask,i,j)){
                        s = b1;
                        m = m << 1 | !(b1 | b2);
                    }else{
                        s = 1;
                        m = m << 1;
                    }
                    send_bit(s);
                    b = b >> 1 | (b1|s)<<7;
                }
                addr.x[j] = b;
                mask.x[j] = m;
                finished = finished & (m==0);
            }
            if(addr.x[0] != 0xff){
                conf->add_device(new onewire_device(addr));
            }
            k++;
            first = false;
        }
    }
}

void onewire_client::run_task(void *param){
    while(1){
        vTaskDelay(pdMS_TO_TICKS(conf->get_read_interval()*1000));
        read_data();
    }
}

onewire_client::onewire_client(onewire_config *ow_conf){
    conf = ow_conf;

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<conf->get_pin());
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    search_devices();
    read_data();
}

void onewire_client::read_data(){
    if(conf->get_num_devices() == 0){
        return;
    }
    portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&myMutex);
    reset_pulse();
    send_byte(0xCC);
    send_byte(0x44);
    portEXIT_CRITICAL(&myMutex);
    while(read_bit() == 0){
        ets_delay_us(100000);
    }
    
    portENTER_CRITICAL(&myMutex);
    reset_pulse();

    uint8_t read_cntr = 0;
    bool chk_crc = false;

    for(int i = 0; i<conf->get_num_devices(); i++){
        do{
            reset_pulse();
            send_byte(0x55);
            onewire_addr_t addr = conf->get_device(i)->get_addr();
            for(int j = 0; j<8; j++){
                send_byte(addr.x[j]);
            }
            send_byte(0xBE);
            onewire_data d;
            for(int j = 0; j<9; j++){
                d.x[j] = read_byte();
            }
            conf->get_device(i)->set_data(d);
            conf->get_device(i)->check_crc();
            chk_crc = conf->get_device(i)->get_crc();
            read_cntr++;
        }while((read_cntr<1) & !chk_crc);
        if(chk_crc){
            ESP_LOGI(TAG, "success after %d tries", read_cntr);
        }else{
            ESP_LOGI(TAG, "failed after %d tries", read_cntr);
        }
        read_cntr = 0;
    }
    portEXIT_CRITICAL(&myMutex);
}

void onewire_client::continous_read(){
    xTaskCreate(run_task, "ONEWIRE_RUN_TASK", 8196, NULL, tskIDLE_PRIORITY+20, &run_task_handle); 
}