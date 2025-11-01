#ifndef WS2812B_HPP
#define WS2812B_HPP

#include "driver/rmt.h"
#include "Base.hpp"

#define LED_NUM 12
#define RMT_NUM 288 // 12*24

/**************************
 *       LED State
 **************************/
enum led_state_t
{
    led_state_wave = 0,
    led_state_rainbow,
    led_state_choose,
    led_state_game
};

/**************************
 *      Color Cycle
 **************************/
inline uint8_t colorCycle_list[][3] = {
    {0, 255, 0},
    {64, 255, 0},
    {128, 255, 0},
    {191, 255, 0},
    {255, 255, 0},
    {255, 191, 0},
    {255, 128, 0},
    {255, 64, 0},
    {255, 0, 0},
    {255, 0, 64},
    {255, 0, 128},
    {255, 0, 191},
    {255, 0, 255},
    {191, 0, 255},
    {128, 0, 255},
    {64, 0, 255},
    {0, 0, 255},
    {0, 64, 255},
    {0, 128, 255},
    {0, 191, 255},
    {0, 255, 255},
    {0, 255, 191},
    {0, 255, 128},
    {0, 255, 64}};

/**************************
 *       LED Class
 **************************/
class LED
{
public:
    led_state_t state;

private:
    rmt_item32_t led_data[RMT_NUM];
    uint8_t current_wave_led[LED_NUM];

public:
    uint8_t luminosity;

public:
    LED(void) : state(led_state_wave), luminosity(0x3C) {}

private:
    bool wave_isnt_zero(void);

public:
    void init(void);
    void update(void);
    void set_one(int led_idx, uint8_t g, uint8_t r, uint8_t b);
    void clear_one(uint8_t num);
    void clear_all(void);
    void close_all(void);
    void set_wave_center(int idx);
    void set_wave_zero(void);
    void run_rainbow(int offset);
    void run_choose(int offset, bool &flag);
    void run_wave(void);
    void bright(uint8_t delta);
    void dim(uint8_t delta);
};

#endif