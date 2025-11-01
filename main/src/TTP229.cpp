#include "TTP229.hpp"

void IRAM_ATTR touch_isr_handler(void *)
{
    BaseType_t xHigherPriorityTaskWoken  = pdTRUE;
    vTaskNotifyGiveFromISR(touch_task_handle, &xHigherPriorityTaskWoken );
    if (xHigherPriorityTaskWoken )
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
    }
}
void Touch::init(void)
{
    gpio_config_t sdo_config = {
        .pin_bit_mask = (1ULL << TOUCH_SDO_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&sdo_config);

    gpio_config_t scl_config = {
        .pin_bit_mask = (1ULL << TOUCH_SCL_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&scl_config);

    gpio_set_level(TOUCH_SCL_GPIO, 1);

    gpio_set_intr_type(TOUCH_SDO_GPIO, GPIO_INTR_NEGEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(TOUCH_SDO_GPIO, touch_isr_handler, NULL);
}
void Touch::read(void)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    uint16_t current_keys = 0;
    for (int i = 0; i < 16; i++)
    {
        gpio_set_level(TOUCH_SCL_GPIO, 0);
        delay_us(5);
        if (gpio_get_level(TOUCH_SDO_GPIO))
        {
            current_keys |= (1 << i);
        }
        gpio_set_level(TOUCH_SCL_GPIO, 1);
        delay_us(5);
    }
    gpio_set_level(TOUCH_SCL_GPIO, 1);
    if (current_keys != previous_keys)
    {
        keys.push(current_keys);
    }
    previous_keys = current_keys;
}