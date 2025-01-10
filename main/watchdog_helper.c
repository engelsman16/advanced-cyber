#include "watchdog_helper.h"

#include "esp_log.h"

#include "log_helper.h"

void watch_dog_task(void *pvParameter)
{
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    ESP_ERROR_CHECK(esp_task_wdt_status(NULL));

    while (1)
    {
        if (has_triggered)
        {
            ESP_LOGE(WATCHDOG_TAG, "TWDT triggered");
            has_triggered = false;
            watch_dog_trigger_count++;
        }

        ESP_ERROR_CHECK(esp_task_wdt_reset());
        vTaskDelay(11000 / portTICK_PERIOD_MS);
    }

    ESP_ERROR_CHECK(esp_task_wdt_delete(NULL));

    vTaskDelete(NULL);
}

esp_err_t watchdog_init(void)
{
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = TWDT_TIMEOUT_MS,
        .idle_core_mask = (1 << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1,
        .trigger_panic = false
    };

    ESP_ERROR_CHECK(esp_task_wdt_init(&twdt_config));
    ESP_LOGI(WATCHDOG_TAG, "TWDT initialized\n");

    return ESP_OK;
}

esp_err_t watchdog_deinit(void)
{
    ESP_ERROR_CHECK(esp_task_wdt_deinit());
    ESP_LOGI(WATCHDOG_TAG, "TWDT deleted\n");

    return ESP_OK;
}