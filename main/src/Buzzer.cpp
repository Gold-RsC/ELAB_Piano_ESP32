#include "Buzzer.hpp"

SignalQueue<uint8_t> notes;

#define __play_music_code(_name)           \
    for (int i = 0; i < numof(_name); ++i) \
    {                                      \
        if (state.reset)                   \
        {                                  \
            state.reset = 0;               \
            i = 0;                         \
        }                                  \
        if (_name[i][0] > 20)              \
        {                                  \
            delay_ms(_name[i][0] - 20);    \
        }                                  \
        else                               \
        {                                  \
            delay_ms(20);                  \
        }                                  \
        if (is_mode_music_pause())         \
        {                                  \
            notes.push(0x80);              \
        }                                  \
        while (is_mode_music_pause())      \
        {                                  \
            delay_ms(100);                 \
        }                                  \
        notes.push(_name[i][1]);           \
    }
#define __play_music_func(_name, _list) \
    play_music_##_name(void)            \
    {                                   \
        __play_music_code(_list);       \
    }

void Buzzer::init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUZZER_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(BUZZER_GPIO, 0);

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = 0};
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = {
            .output_invert = 0}};
    ledc_channel_config(&ledc_channel);

    for (int pitch = 0; pitch < 128; ++pitch)
    {
        freq_pitch_map[pitch] = uint32_t(440 * pow(2.0, (pitch - 69) / 12.0));
    }
}
void Buzzer::set_pitch(uint8_t _pitch)
{
    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq_pitch_map[_pitch]);
}
bool Buzzer::is_mode_init(void)
{
    return state.mode == buzzer_mode_init;
}
bool Buzzer::is_mode_choose(void)
{
    return state.mode == buzzer_mode_choose;
}
bool Buzzer::is_mode_music_on(void)
{
    return state.mode == buzzer_mode_music_on;
}
bool Buzzer::is_mode_music_pause(void)
{
    return state.mode == buzzer_mode_music_pause;
}
bool Buzzer::is_mode_game(void)
{
    return state.mode == buzzer_mode_game;
}
bool Buzzer::is_state_off(void)
{
    return !state.onoff;
}
bool Buzzer::is_state_on(void)
{
    return state.onoff;
}

void Buzzer::buzzer_on(void)
{
    state.onoff = 1;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 128);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
void Buzzer::buzzer_off(void)
{
    state.onoff = 0;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void Buzzer::__play_music_func(next, music_list_next);
void Buzzer::__play_music_func(previous, music_list_previous);
void Buzzer::__play_music_func(1, music_list_1);
void Buzzer::__play_music_func(wrong, music_list_wrong);
void Buzzer::__play_music_func(test, music_list_test);
void Buzzer::__play_music_func(game, music_list_game);
void Buzzer::__play_music_func(animation, music_list_animation);

