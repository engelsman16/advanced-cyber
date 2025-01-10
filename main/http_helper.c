#include "http_helper.h"

#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_chip_info.h"
#include "esp_efuse.h"
#include "watchdog_helper.h"
#include "nvs_helper.h"


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) 
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(HTTPS_TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(HTTPS_TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(HTTPS_TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(HTTPS_TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(HTTPS_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client)) 
        {
            printf("%.*s", evt->data_len, (char*)evt->data);
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(HTTPS_TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(HTTPS_TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(HTTPS_TAG, "HTTP_EVENT_ON_REDIRECT");
        break;
    }
    return ESP_OK;
}

void https_get_task(void *pvParameters)
{
    const char *cert_pem = (const char *)pvParameters;

    esp_http_client_config_t config = 
    {
        .url = "https://google.com",
        .event_handler = _http_event_handler,
        .cert_pem = cert_pem,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);

    if (err != ESP_OK) 
    {
        ESP_LOGE(HTTPS_TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

static esp_err_t https_post(const char *url, const char *post_data, const char *auth_header)
{
    esp_http_client_config_t config = {
        .url = url,
        .cert_pem = NULL,                      
        .skip_cert_common_name_check = true,  
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(HTTPS_TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }

    char full_auth_header[128];
    snprintf(full_auth_header, sizeof(full_auth_header), "ApiKey %s", auth_header);


    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Authorization", full_auth_header);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(HTTPS_TAG, "HTTP POST Status = %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(HTTPS_TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

void https_post_task(void *pvParameters)
{
    static const char *url = "https://20.90.144.160:9200/_bulk?pretty";

    while (1) {
        uint32_t free_heap = esp_get_free_heap_size();
        int watchdog_count = watch_dog_trigger_count;

        wifi_ap_record_t ap_info;
        char ssid[33] = "N/A";
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) 
        {
        strncpy(ssid, (char *)ap_info.ssid, sizeof(ssid) - 1);
        ssid[sizeof(ssid) - 1] = '\0'; 
        }

        char *placeholder;
        esp_err_t nvs_error = load_nvs_value("storage", "apikey", &placeholder);

        char *nvs_status = "NVS-OK\0";
        if (nvs_error != ESP_OK) {
            nvs_status = "NVS-ERROR\0";
        }
        else {
            nvs_status = "NVS-OK\0";
        }

        char flash_encrypt[20] = "FLASH-ENCRYPTED\0";
        char secure_boot[20] = "SECURE_BOOT_ENABLED\0";

        char post_data[512]; 
        snprintf(post_data, sizeof(post_data),
        "{\"index\": {\"_index\": \"esp\"}}\n"
        "{\"free_heap\": %ld, \"watchdog_count\": %d, \"latest_nvs_status\": \"%s\", "
        "\"wifi_ssid\": \"%s\", \"secure_boot\": \"%s\", \"flash_encrypt\": \"%s\"}\n",
        free_heap, watchdog_count, nvs_status, ssid, secure_boot, flash_encrypt);

        if (https_post(url, post_data, placeholder) != ESP_OK) {
            ESP_LOGE(HTTPS_TAG, "HTTPS POST task failed");
        }

        free(placeholder);

        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }

    vTaskDelete(NULL);
}