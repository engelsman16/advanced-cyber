#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"


#include "lwip/err.h"
#include "lwip/sys.h"

#include "nvs_helper.h"
#include "log_helper.h"
#include "http_helper.h"
#include "wifi_helper.h"
#include "watchdog_helper.h"

bool has_triggered = false;
int watch_dog_trigger_count = 0;

void esp_task_wdt_isr_user_handler(void)
{
    has_triggered = true;
}

void app_main(void)
{
    ESP_ERROR_CHECK(watchdog_init());
    ESP_ERROR_CHECK(nvs_init());

    esp_err_t wifi_status = WIFI_FAILURE;

    char* password;
    char* ssid;
    char* my_cert;
    char* apikey;

    if (load_nvs_value("storage", "password", &password) != ESP_OK || load_nvs_value("storage", "ssid", &ssid) != ESP_OK || load_nvs_value("certs", "cert", &my_cert) != ESP_OK || load_nvs_value("storage", "apikey", &apikey) != ESP_OK)
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

    xTaskCreate(&https_post_task, "https_post_task", 8192, NULL, 5, NULL);    

    xTaskCreate(&watch_dog_task, "watch_dog_task", 2048, NULL, 5, NULL);

    free(password);
    free(ssid);
    free(my_cert);
    free(apikey);

    vTaskDelay(600000 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(watchdog_deinit());
    ESP_ERROR_CHECK(nvs_flash_deinit());
}