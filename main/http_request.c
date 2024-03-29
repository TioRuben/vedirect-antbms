#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include "esp_event.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "watchdog.h"

#define URL CONFIG_EMONCMS_URL

#define TAG "HTTPS_REQUEST"

char currentRequest[512];
char requestQueue[512];
bool pendingQueue = false;

bool enqueueRequest(char *newRequest)
{
    if (pendingQueue)
    {
        return false;
    }
    else
    {
        strcpy(requestQueue, newRequest);
        pendingQueue = true;
    }
    return true;
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    default:
        break;
    }
    return ESP_OK;
}

void vHTTPSRequest(void *pvParameters)
{
    while (1)
    {
        if (pendingQueue)
        {
            strcpy(currentRequest, requestQueue);
            ESP_LOGI(TAG, "Sending %s", currentRequest);
            pendingQueue = false;
            esp_http_client_config_t config = {
                .url = URL,
                .event_handler = _http_event_handler,
                .method = HTTP_METHOD_POST,
                .crt_bundle_attach = esp_crt_bundle_attach,
            };
            esp_http_client_handle_t client = esp_http_client_init(&config);
            esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
            esp_http_client_set_post_field(client, currentRequest, strlen(currentRequest));
            esp_err_t err = esp_http_client_perform(client);
            if (err == ESP_OK)
            {
                ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %lld",
                         esp_http_client_get_status_code(client),
                         esp_http_client_get_content_length(client));
            }
            else
            {
                ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
            }
            esp_http_client_cleanup(client);
            if (strstr(currentRequest, "mppt1") != NULL)
            {
                set_sent(1);
            }
            if (strstr(currentRequest, "mppt2") != NULL)
            {
                set_sent(2);
            }
            if (strstr(currentRequest, "bms") != NULL)
            {
                set_sent(3);
            }
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}