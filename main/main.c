#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define MAX_FAILURES 10

static EventGroupHandle_t wifi_event_group;

static int s_retry_num = 0;

static const char *WIFI_TAG = "WIFI-LOGGER";
static const char *NVS_TAG = "NVS-LOGGER";


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI(WIFI_TAG, "Connecting to AP...");
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (s_retry_num < MAX_FAILURES)
		{
			ESP_LOGI(WIFI_TAG, "Reconnecting to AP...");
			esp_wifi_connect();
			s_retry_num++;
		} else {
			xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
		}
	}
}

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIFI_TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }

}

esp_err_t connect_wifi(char* ssid, char* password)
{
	int status = WIFI_FAILURE;

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t wifi_handler_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &wifi_handler_event_instance));

    esp_event_handler_instance_t got_ip_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &ip_event_handler,
                                                        NULL,
                                                        &got_ip_event_instance));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIFI_TAG, "STA initialization complete");

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
            WIFI_SUCCESS | WIFI_FAILURE,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_SUCCESS) {
        ESP_LOGI(WIFI_TAG, "Connected to ap");
        status = WIFI_SUCCESS;
    } else if (bits & WIFI_FAILURE) {
        ESP_LOGW(WIFI_TAG, "Failed to connect to ap");
        status = WIFI_FAILURE;
    } else {
        ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
        status = WIFI_FAILURE;
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(wifi_event_group);

    return status;
}

esp_err_t nvs_init()
{
    esp_err_t status = ESP_OK;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);

    return status;
}

esp_err_t read_wifi_credentials_from_nvs(char* nvs_namespace, char* ssid, char* password, size_t ssid_size, size_t password_size)
{
    nvs_handle_t nvs_handle;
    esp_err_t status = ESP_OK;

    esp_err_t err = nvs_open(nvs_namespace, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(NVS_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    err = nvs_get_str(nvs_handle, "ssid", ssid, &ssid_size);

    if (err != ESP_OK)
    {
        ESP_LOGE(NVS_TAG, "Error (%s) reading ssid from NVS!", esp_err_to_name(err));
        status = err;
    }
    err = nvs_get_str(nvs_handle, "password", password, &password_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(NVS_TAG, "Error (%s) reading password from NVS!", esp_err_to_name(err));
        status = err;
    }

    if (strlen(ssid) > 0 && strlen(password) > 0)
    {
        ESP_LOGI(NVS_TAG, "Successfully read ssid and password from NVS");
    }

    nvs_close(nvs_handle);

    return status;
}

void app_main(void)
{
	esp_err_t wifi_status = WIFI_FAILURE;

    ESP_ERROR_CHECK(nvs_init());

    char ssid[32];
    char password[64];

    ESP_ERROR_CHECK(read_wifi_credentials_from_nvs("wifi_storage", ssid, password, sizeof(ssid), sizeof(password)));

	wifi_status = connect_wifi(ssid, password);
    if (wifi_status != WIFI_SUCCESS)
	{
        ESP_LOGE(WIFI_TAG, "Failed to associate to AP, terminating...");
        esp_restart();
	}
}