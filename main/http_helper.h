#ifndef HTTP_HELPER_H
#define HTTP_HELPER_H

#include "esp_http_client.h"
#include "esp_log.h"

#include "log_helper.h"

esp_err_t _http_event_handler(esp_http_client_event_t *evt);

void https_get_task(void *pvParameters);

#endif // HTTP_HELPER_H