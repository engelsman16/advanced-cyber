#include "nvs_helper.h"

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

char * nvs_load_value_if_exist(nvs_handle handle, const char* key)
{
    size_t value_size;

    if(nvs_get_str(handle, key, NULL, &value_size) != ESP_OK)
    {
        ESP_LOGE(NVS_TAG, "Failed to get size of key: %s", key);
        return NULL;
    }

    char* value = malloc(value_size);

    if(nvs_get_str(handle, key, value, &value_size) != ESP_OK)
    {
        ESP_LOGE(NVS_TAG, "Failed to load key: %s", key);
        free(value);
        return NULL;
    }

    return value;
}

esp_err_t load_nvs_value(const char* nvs_namespace, const char* key, char** value)
{
    nvs_handle handle;
    esp_err_t err;

    err = nvs_open(nvs_namespace, NVS_READONLY, &handle);

    if (err != ESP_OK) 
    {
        ESP_LOGE(NVS_TAG, "Error (%s) opening NVS handle for %s!", esp_err_to_name(err), nvs_namespace);
        return err;
    }

    *value = nvs_load_value_if_exist(handle, key);
    if (*value == NULL) 
    {
        ESP_LOGE(NVS_TAG, "Failed to load key: %s from namespace: %s", key, nvs_namespace);
        return ESP_FAIL;
    }

    nvs_close(handle);
    return ESP_OK;
}