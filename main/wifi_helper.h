#ifndef wifi_helper_h
#define wifi_helper_h

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "log_helper.h"

#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define MAX_FAILURES 10

esp_err_t connect_wifi(const char* ssid, const char* password);

#endif // wifi_helper_h


