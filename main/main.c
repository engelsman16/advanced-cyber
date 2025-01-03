#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "nvs_helper.h"
#include "log_helper.h"
#include "http_helper.h"
#include "wifi_helper.h"

void app_main(void)
{
    esp_err_t wifi_status = WIFI_FAILURE;

    ESP_ERROR_CHECK(nvs_init());

    char* password;
    char* ssid;
    char* my_cert;

    if (load_nvs_value("wifi_storage", "password", &password) != ESP_OK || load_nvs_value("wifi_storage", "ssid", &ssid) != ESP_OK || load_nvs_value("certs", "cert", &my_cert) != ESP_OK) 
    {
        ESP_LOGE("ERROR", "Failed to load from NVS");
        esp_restart();
    }

    wifi_status = connect_wifi(ssid, password);

    if (wifi_status == WIFI_FAILURE) 
    {
        ESP_LOGE(WIFI_TAG, "Failed to connect to wifi");
        esp_restart();
    }

    xTaskCreate(&https_get_task, "https_get_task", 8192, (void*)my_cert, 5, NULL);

    free(password);
    free(ssid);
    free(my_cert);
}