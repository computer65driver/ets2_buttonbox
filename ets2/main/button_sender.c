#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "lwip/sockets.h"

#define GPIO_ENGINE 18
#define GPIO_HORN   15
#define GPIO_TOGGLE 21
#define GPIO_LIGHTS 5


#define SERVER_IP "IP_ADDRESS" 
#define PORT 1234

#pragma pack(push, 1) 
typedef struct {
    uint8_t engine_state; 
    uint8_t horn;
    uint8_t toggle;
    uint8_t light_state;
} button_packet_t;
#pragma pack(pop)



static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* d = (wifi_event_sta_disconnected_t*) event_data;
        printf("WiFi Error! Reason: %d. Restarting...\n", d->reason);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("IP has been just taken: " IPSTR "\n", IP2STR(&event->ip_info.ip));
    }
}

void wifi_init_sta() {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "*********",     
            .password = "*********", 
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

void udp_sender_task(void *pvParameters) {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    button_packet_t packet = {0};
    
  
    uint8_t last_engine = 1, last_horn = 1, last_toggle = 1, last_light = 1;

    while (1) {
        uint8_t cur_engine = gpio_get_level(GPIO_ENGINE);
        uint8_t cur_horn   = gpio_get_level(GPIO_HORN);
        uint8_t cur_toggle = gpio_get_level(GPIO_TOGGLE);
        uint8_t cur_light  = gpio_get_level(GPIO_LIGHTS);


        if (cur_engine != last_engine || cur_horn != last_horn || 
            cur_toggle != last_toggle || cur_light != last_light ) { 
            
            packet.engine_state = (cur_engine == 0) ? 1 : 0;
            packet.horn         = (cur_horn == 0) ? 1 : 0;
            packet.toggle       = (cur_toggle == 0) ? 1 : 0;
            packet.light_state  = (cur_light == 0) ? 1 : 0;

            
            sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            
            

            // Update the states
            last_engine = cur_engine;
            last_horn = cur_horn;
            last_toggle = cur_toggle;
            last_light = cur_light;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    gpio_config_t io_conf = {};
    
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_ENGINE) | (1ULL << GPIO_HORN) | 
                           (1ULL << GPIO_TOGGLE) | (1ULL << GPIO_LIGHTS);
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);



    gpio_install_isr_service(0);

    wifi_init_sta();

    vTaskDelay(pdMS_TO_TICKS(5000));

    xTaskCreate(udp_sender_task, "udp_task", 4096, NULL, 5, NULL);

}
