#ifndef WATCHDOG_HELPER_H
#define WATCHDOG_HELPER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

#define TWDT_TIMEOUT_MS 3000

extern bool has_triggered;

void watch_dog_task(void *pvParameter);

esp_err_t watchdog_init(void);
esp_err_t watchdog_deinit(void);

#endif // WATCHDOG_HELPER_H