idf_component_register(SRCS 
                      "emoncms_request.c" 
                      "http_request.c"
                      "wifi.c" 
                      "mppt.c" 
                      "watchdog.c" 
                      "bluetooth.c" 
                      "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES vedirect antbms driver mbedtls freertos esp_http_client
                    PRIV_REQUIRES nvs_flash bt)
