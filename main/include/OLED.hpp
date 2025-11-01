#ifndef OLED_HPP
#define OLED_HPP
#include "driver/spi_master.h"
#include "Base.hpp"
#include "font.hpp"
/**************************
 *       Screen x-y
 **************************/
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/**************************
 *       Screen Cmd
 **************************/
#define SCREEN_SETDISPLAYON 0xAF
#define SCREEN_SETDISPLAYOFF 0xAE
#define SCREEN_MEMORYMODE 0x20
#define SCREEN_COLUMNADDR 0x21
#define SCREEN_PAGEADDR 0x22
#define SCREEN_SETSTARTLINE 0x40
#define SCREEN_DISPLAYALLON_RESUME 0xA4
#define SCREEN_DISPLAYALLON 0xA5
#define SCREEN_NORMALDISPLAY 0xA6
#define SCREEN_INVERTDISPLAY 0xA7
#define SCREEN_SETMULTIPLEX 0xA8
#define SCREEN_SETCONTRAST 0x81
#define SCREEN_CHARGEPUMP 0x8D
#define SCREEN_EXTERNALVCC 0x1
#define SCREEN_SWITCHCAPVCC 0x2
#define SCREEN_SEGREMAP 0xA0
#define SCREEN_COMSCANDEC 0xC8
#define SCREEN_COMSCANINC 0xC0
#define SCREEN_SETDISPLAYOFFSET 0xD3
#define SCREEN_SETDISPLAYCLOCKDIV 0xD5
#define SCREEN_SETPRECHARGE 0xD9
#define SCREEN_SETCOMPINS 0xDA

/**************************
 *      Screen Class
 **************************/
class Screen
{
public:
private:
    spi_device_handle_t spi;
    uint8_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT / 8];

public:
    Screen(void) {}

public:
    void init(void);

private:
    void write_cmd(uint8_t cmd);
    void write_data(const uint8_t *data, size_t len);
    void draw_rectangle_px(int x, int y, int width, int height, int color);
    void draw_word_px(const uint8_t *map, uint8_t x, uint8_t y, uint8_t width = 16, uint8_t height = 16);
    void draw_word_word(const uint8_t *map, uint8_t x, uint8_t y, uint8_t width = 2, uint8_t height = 1);

public:
    void invert_px(uint8_t x, uint8_t y, uint8_t width = 16, uint8_t height = 16);
    void invert_word(uint8_t x, uint8_t y, uint8_t width = 2, uint8_t height = 1);
    void display_init(void);
    void display_setting(void);
    void display_led(void);
    void display_buzzer(void);
    void display_music_on(void);
    void display_music_pause(void);
    void display_music_start(void);
    void display_game(void);
    void display_game_perfect(void);
    void display_game_good(void);
    void display_game_bad(void);
    void display_game_miss(void);
    void display_game_end(uint64_t max_combo);
    void play_boot_animation(void);
    void add_asterisk_word(uint8_t x, uint8_t y);
    void add_sharp_word(uint8_t x, uint8_t y);
    void add_connect_word(uint8_t x, uint8_t y);
    void add_plus_word(uint8_t x, uint8_t y);
    void add_slash_word(uint8_t x, uint8_t y);
    void add_char(uint8_t byte, uint8_t x, uint8_t y);
    void add_words(const char *str, uint8_t x, uint8_t y);
    void add_hex(uint8_t num, uint8_t x, uint8_t y);
    void add_num_unsigned(uint8_t num, uint8_t x, uint8_t y);
    void add_num_signed(int8_t num, uint8_t x, uint8_t y);
    void add_num(int8_t num, uint8_t x, uint8_t y);
    void add_pitch(uint8_t pitch, uint8_t x, uint8_t y);
    void add_pitch_line(uint8_t pitch, uint8_t x, uint8_t y);
    void add_yinyu_line(uint8_t offset, uint8_t x, uint8_t y);
    void add_lumin_line(uint8_t lumin, uint8_t x, uint8_t y);
    void clear_px(uint8_t x, uint8_t y, uint8_t width = 16, uint8_t height = 16);
    void clear_all(void);
    void clear_word(uint8_t x, uint8_t y, uint8_t width = 2, uint8_t height = 1);
    void clear_main(void);
    void clear_main_line1(void);
    void clear_main_line2(void);
    void clear_head(void);
    void clear_tail(void);
    void update(void);
};

#endif
