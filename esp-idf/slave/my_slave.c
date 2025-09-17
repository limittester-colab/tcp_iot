#include "esp_system.h"
#include "esp_log.h"
// This example needs rtc driver to read button state
#include "driver/rtc_io.h"
#include "driver/gpio.h"
#include "espnow_basic_config.h"

static const char *TAG = "My_Slave";
static int iter;
// Your function to populate a my_data_t to send
void my_data_populate(my_data_t *data)
{
    ESP_LOGI(TAG, "Populating my_data_t");
    data->random_value = iter++;
    data->button_pushed = (gpio_get_level(GPIO_NUM_0) == 0);
}
