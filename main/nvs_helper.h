#ifndef NVS_HELPER_H
#define NVS_HELPER_H

#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "log_helper.h"

esp_err_t nvs_init();

char * nvs_load_value_if_exist(nvs_handle handle, const char* key);

esp_err_t load_nvs_value(const char* nvs_namespace, const char* key, char** value);



#endif // NVS_HELPER_H