idf_component_register(SRCS "button_sender.c"
                       PRIV_REQUIRES spi_flash nvs_flash esp_wifi esp_netif lwip driver freertos
                       INCLUDE_DIRS "")
