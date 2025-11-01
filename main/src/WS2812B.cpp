#include "WS2812B.hpp"
bool LED::wave_isnt_zero(void)
{
    for (int i = 0; i < LED_NUM; ++i)
    {
        if (current_wave_led[i])
        {
            return true;
        }
    }
    return false;
}

void LED::init(void)
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(LED_GPIO, RMT_CHANNEL_0);
    config.clk_div = 2;
    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);

    close_all();
}

void LED::update(void)
{
    rmt_write_items(RMT_CHANNEL_0, led_data, RMT_NUM, true);
}
void LED::set_one(int led_idx, uint8_t g, uint8_t r, uint8_t b)
{
    uint32_t color = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;

    for (int bit = 0; bit < 24; bit++)
    {
        int idx = (11 - led_idx) * 24 + bit;

        if (color & (1 << (23 - bit)))
        {
            led_data[idx].level0 = 1;
            led_data[idx].duration0 = 36;
            led_data[idx].level1 = 0;
            led_data[idx].duration1 = 14;
        }
        else
        {
            led_data[idx].level0 = 1;
            led_data[idx].duration0 = 14;
            led_data[idx].level1 = 0;
            led_data[idx].duration1 = 36;
        }
    }
}
void LED::clear_one(uint8_t num)
{
    set_one(num, 0, 0, 0);
    update();
}
void LED::clear_all(void)
{
    for (int i = 0; i < LED_NUM; i++)
    {
        set_one(i, 0, 0, 0);
    }
}
void LED::close_all(void)
{
    clear_all();
    update();
}

void LED::set_wave_center(int idx)
{
    current_wave_led[idx] = 0xFF;
}
void LED::set_wave_zero(void)
{
    memset(current_wave_led, 0, sizeof(uint8_t) * LED_NUM);
}

void LED::run_rainbow(int offset)
{
    for (int i = 0; i < LED_NUM; i++)
    {
        int idx = (i + offset) % numof(colorCycle_list);
        set_one(i, (uint32_t)colorCycle_list[idx][0] * luminosity / 0xFF, (uint32_t)colorCycle_list[idx][1] * luminosity / 0xFF, (uint32_t)colorCycle_list[idx][2] * luminosity / 0xFF);
    }
    update();
}
void LED::run_choose(int offset, bool &flag)
{
    if (wave_isnt_zero())
    {
        uint8_t previous_wave_led[LED_NUM] = {0};
        // 0
        previous_wave_led[0] = clamp(previous_wave_led[0] + current_wave_led[0] / 2, 0, 255);
        previous_wave_led[1] = clamp(previous_wave_led[1] + current_wave_led[0] / 5, 0, 255);
        // 1-14
        for (int i = 1; i < LED_NUM - 1; ++i)
        {
            previous_wave_led[i - 1] = clamp(previous_wave_led[i - 1] + current_wave_led[i] / 5, 0, 255);
            previous_wave_led[i] = clamp(previous_wave_led[i] + current_wave_led[i] / 2, 0, 255);
            previous_wave_led[i + 1] = clamp(previous_wave_led[i + 1] + current_wave_led[i] / 5, 0, 255);
        }
        // 15
        previous_wave_led[LED_NUM - 2] = clamp(previous_wave_led[LED_NUM - 2] + current_wave_led[LED_NUM - 1] / 5, 0, 255);
        previous_wave_led[LED_NUM - 1] = clamp(previous_wave_led[LED_NUM - 1] + current_wave_led[LED_NUM - 1] / 2, 0, 255);

        memcpy(current_wave_led, previous_wave_led, LED_NUM * sizeof(uint8_t));
        // out
        for (int i = 0; i < LED_NUM; ++i)
        {
            int idx = (i + offset) % numof(colorCycle_list);
            set_one(i, (uint32_t)colorCycle_list[idx][0] * current_wave_led[i] * luminosity / 0xFF / 0xFF, (uint32_t)colorCycle_list[idx][1] * current_wave_led[i] * luminosity / 0xFF / 0xFF, (uint32_t)colorCycle_list[idx][2] * current_wave_led[i] * luminosity / 0xFF / 0xFF);
        }
        flag = 1;
        update();
    }
    else
    {
        if (!(offset * 3 % numof(colorCycle_list)))
        {
            if (flag)
            {
                set_one(0, luminosity, luminosity, luminosity);
                set_one(11, luminosity, luminosity, luminosity);
            }
            else
            {
                set_one(0, 0, 0, 0);
                set_one(11, 0, 0, 0);
            }
            flag = !flag;
            update();
        }
    }
}
void LED::run_wave(void)
{
    if (wave_isnt_zero())
    {
        uint8_t previous_wave_led[LED_NUM] = {0};
        // 0
        previous_wave_led[0] = clamp(previous_wave_led[0] + current_wave_led[0] / 2, 0, 255);
        previous_wave_led[1] = clamp(previous_wave_led[1] + current_wave_led[0] / 5, 0, 255);
        // 1-14
        for (int i = 1; i < LED_NUM - 1; ++i)
        {
            previous_wave_led[i - 1] = clamp(previous_wave_led[i - 1] + current_wave_led[i] / 5, 0, 255);
            previous_wave_led[i] = clamp(previous_wave_led[i] + current_wave_led[i] / 2, 0, 255);
            previous_wave_led[i + 1] = clamp(previous_wave_led[i + 1] + current_wave_led[i] / 5, 0, 255);
        }
        // 15
        previous_wave_led[LED_NUM - 2] = clamp(previous_wave_led[LED_NUM - 2] + current_wave_led[LED_NUM - 1] / 5, 0, 255);
        previous_wave_led[LED_NUM - 1] = clamp(previous_wave_led[LED_NUM - 1] + current_wave_led[LED_NUM - 1] / 2, 0, 255);

        memcpy(current_wave_led, previous_wave_led, LED_NUM * sizeof(uint8_t));
        // out
        for (int i = 0; i < LED_NUM; ++i)
        {
            set_one(i, (uint32_t)current_wave_led[i] * luminosity / 0xFF, (uint32_t)current_wave_led[i] * luminosity / 0xFF, (uint32_t)current_wave_led[i] * luminosity / 0xFF);
        }
        update();
    }
}

void LED::bright(uint8_t delta)
{
    if ((0xFF - luminosity) > delta)
    {
        luminosity += delta;
    }
    else
    {
        luminosity = 0xFF;
    }
}
void LED::dim(uint8_t delta)
{
    if (luminosity > delta)
    {
        luminosity -= delta;
    }
    else
    {
        luminosity = 0;
    }
}
