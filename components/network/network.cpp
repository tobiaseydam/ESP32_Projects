#include "network.hpp"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "eth_phy/phy_lan8720.h"
#include "esp_event_loop.h"

#include <string.h>

network_config::network_config(){
    event_group = xEventGroupCreate();
}

wifi_config::wifi_config():network_config(){

}

eth_config::eth_config():network_config(){
    
}

esp_err_t network_adapter::event_handler(void *ctx, system_event_t *event){
    switch(event->event_id) {
        case SYSTEM_EVENT_WIFI_READY:{              /**< ESP32 WiFi ready */
            ESP_LOGI(TAG, "SYSTEM_EVENT_WIFI_READY");
            break;
        }

        case SYSTEM_EVENT_SCAN_DONE:{               /**< ESP32 finish scanning AP */
            ESP_LOGI(TAG, "SYSTEM_EVENT_SCAN_DONE");
            break;
        }

        case SYSTEM_EVENT_STA_START:{               /**< ESP32 station start */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            wifi_config* conf = (wifi_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), 0xFF);
            xEventGroupSetBits(conf->get_event_group(), START_BIT);
            break;
        }

        case SYSTEM_EVENT_STA_STOP:{                /**< ESP32 station stop */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_STOP");
            wifi_config* conf = (wifi_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), 0xFF);
            xEventGroupSetBits(conf->get_event_group(), STOP_BIT);
            break;
        }

        case SYSTEM_EVENT_STA_CONNECTED:{           /**< ESP32 station connected to AP */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_CONNECTED");
            wifi_config* conf = (wifi_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), DISCONNECTED_BIT);
            xEventGroupSetBits(conf->get_event_group(), CONNECTED_BIT);
            break;
        }

        case SYSTEM_EVENT_STA_DISCONNECTED:{        /**< ESP32 station disconnected from AP */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            wifi_config* conf = (wifi_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), CONNECTED_BIT);
            xEventGroupSetBits(conf->get_event_group(), DISCONNECTED_BIT);
            break;
        }

        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:{     /**< the auth mode of AP connected by ESP32 station changed */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_AUTHMODE_CHANGE");
            break;
        }

        case SYSTEM_EVENT_STA_GOT_IP:{               /**< ESP32 station got IP from connected AP */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            wifi_config* conf = (wifi_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), LOST_IP_BIT);
            xEventGroupSetBits(conf->get_event_group(), GOT_IP_BIT);
            break;
        }

        case SYSTEM_EVENT_STA_LOST_IP:{              /**< ESP32 station lost IP and the IP is reset to 0 */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_LOST_IP");
            wifi_config* conf = (wifi_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), GOT_IP_BIT);
            xEventGroupSetBits(conf->get_event_group(), LOST_IP_BIT);
            break;
        }

        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:{      /**< ESP32 station wps succeeds in enrollee mode */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_WPS_ER_SUCCESS");
            break;
        }

        case SYSTEM_EVENT_STA_WPS_ER_FAILED:{       /**< ESP32 station wps fails in enrollee mode */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_WPS_ER_FAILED");
            break;
        }

        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:{      /**< ESP32 station wps timeout in enrollee mode */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_WPS_ER_TIMEOUT");
            break;
        }

        case SYSTEM_EVENT_STA_WPS_ER_PIN:{          /**< ESP32 station wps pin code in enrollee mode */
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_WPS_ER_PIN");
            break;
        }

        case SYSTEM_EVENT_AP_START:{                /**< ESP32 soft-AP start */
            ESP_LOGI(TAG, "SYSTEM_EVENT_AP_START");
            wifi_config* conf = (wifi_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), 0xFF);
            xEventGroupSetBits(conf->get_event_group(), START_BIT);
            break;
        }

        case SYSTEM_EVENT_AP_STOP:{                 /**< ESP32 soft-AP stop */
            ESP_LOGI(TAG, "SYSTEM_EVENT_AP_STOP");
            wifi_config* conf = (wifi_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), 0xFF);
            xEventGroupSetBits(conf->get_event_group(), STOP_BIT);
            break;
        }

        case SYSTEM_EVENT_AP_STACONNECTED:{         /**< a station connected to ESP32 soft-AP */
            ESP_LOGI(TAG, "SYSTEM_EVENT_AP_STACONNECTED");
            break;
        }

        case SYSTEM_EVENT_AP_STADISCONNECTED:{      /**< a station disconnected from ESP32 soft-AP */
            ESP_LOGI(TAG, "SYSTEM_EVENT_AP_STADISCONNECTED");
            break;
        }

        case SYSTEM_EVENT_AP_STAIPASSIGNED:{        /**< ESP32 soft-AP assign an IP to a connected station */
            ESP_LOGI(TAG, "SYSTEM_EVENT_AP_STAIPASSIGNED");
            break;
        }

        case SYSTEM_EVENT_AP_PROBEREQRECVED:{       /**< Receive probe request packet in soft-AP interface */
            ESP_LOGI(TAG, "SYSTEM_EVENT_AP_PROBEREQRECVED");
            break;
        }

        case SYSTEM_EVENT_GOT_IP6:{                 /**< ESP32 station or ap or ethernet interface v6IP addr is preferred */
            ESP_LOGI(TAG, "SYSTEM_EVENT_GOT_IP6");
            break;
        }

        case SYSTEM_EVENT_ETH_START:{               /**< ESP32 ethernet start */
            ESP_LOGI(TAG, "SYSTEM_EVENT_ETH_START");
            eth_config* conf = (eth_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), 0xFF);
            xEventGroupSetBits(conf->get_event_group(), START_BIT);
            break;
        }

        case SYSTEM_EVENT_ETH_STOP:{                /**< ESP32 ethernet stop */
            ESP_LOGI(TAG, "SYSTEM_EVENT_ETH_STOP");
            eth_config* conf = (eth_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), 0xFF);
            xEventGroupSetBits(conf->get_event_group(), STOP_BIT);
            break;
        }

        case SYSTEM_EVENT_ETH_CONNECTED:{            /**< ESP32 ethernet phy link up */
            ESP_LOGI(TAG, "SYSTEM_EVENT_ETH_CONNECTED");
            eth_config* conf = (eth_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), DISCONNECTED_BIT);
            xEventGroupSetBits(conf->get_event_group(), CONNECTED_BIT);
            break;
        }

        case SYSTEM_EVENT_ETH_DISCONNECTED:{         /**< ESP32 ethernet phy link down */
            ESP_LOGI(TAG, "SYSTEM_EVENT_ETH_DISCONNECTED");
            eth_config* conf = (eth_config*) ctx;
            xEventGroupClearBits(conf->get_event_group(), CONNECTED_BIT);
            xEventGroupSetBits(conf->get_event_group(), DISCONNECTED_BIT);
            break;
        }

        case SYSTEM_EVENT_ETH_GOT_IP:{               /**< ESP32 ethernet got IP from connected AP */
            ESP_LOGI(TAG, "SYSTEM_EVENT_ETH_GOT_IP");
            eth_config* conf = (eth_config*) ctx;
            xEventGroupSetBits(conf->get_event_group(), GOT_IP_BIT);
            break;
        }

        case SYSTEM_EVENT_MAX:{
            ESP_LOGI(TAG, "SYSTEM_EVENT_MAX");
            break;
        }
    }
    return ESP_OK;
}

void network_adapter::eth_gpio_config_rmii(){
    phy_rmii_configure_data_interface_pins();
    phy_rmii_smi_configure_pins(23, 18);
}

void network_adapter::run_task(void *param){
    eth_config_t config;
    memset(&config, 0, sizeof(config)); 
    config.phy_addr = (eth_phy_base_t)0;
    config.mac_mode = ETH_MODE_RMII;
    config.clock_mode = ETH_CLOCK_GPIO0_IN;
    config.flow_ctrl_enable = true;
    config.phy_init = phy_lan8720_init;
    config.phy_check_init = phy_lan8720_check_phy_init;
    config.phy_power_enable = phy_lan8720_power_enable;
    config.phy_check_link = phy_mii_check_link_status;
    config.phy_get_speed_mode = phy_lan8720_get_speed_mode;
    config.phy_get_duplex_mode = phy_lan8720_get_duplex_mode;
    config.phy_get_partner_pause_enable = phy_mii_get_partner_pause_enable;
    config.gpio_config = eth_gpio_config_rmii;
    config.tcpip_input = tcpip_adapter_eth_input;
    ESP_ERROR_CHECK(esp_eth_init(&config));
    ESP_ERROR_CHECK(esp_eth_enable());

    while(1){

    }

}

network_adapter::network_adapter(wifi_config* config){
    conf = config;
}

network_adapter::network_adapter(eth_config* config){
    conf = config;
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, conf));
}

void network_adapter::connect(){
    xTaskCreate(run_task, "NETWORK_RUN_TASK", 8196, conf, tskIDLE_PRIORITY, &run_task_handle); 
};