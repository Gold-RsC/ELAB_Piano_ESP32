#include "OLED.hpp"
void Screen::init(void)
{
    // 配置SPI总线
    spi_bus_config_t buscfg = {
        .mosi_io_num = SCREEN_MOSI_GPIO,
        .miso_io_num = -1, // SPI 4线模式通常不需要MISO:cite[1]
        .sclk_io_num = SCREEN_CLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SCREEN_WIDTH * SCREEN_HEIGHT / 8};

    // 配置SPI设备
    spi_device_interface_config_t devcfg = {
        .mode = 0,                          // SPI模式0:cite[1]
        .clock_speed_hz = 10 * 1000 * 1000, // 时钟频率，例如10MHz
        .spics_io_num = SCREEN_CS_GPIO,
        .queue_size = 7,
    };

    // 初始化SPI总线
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    // 添加设备到SPI总线
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);

    // 配置DC和RST引脚为GPIO输出模式
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SCREEN_DC_GPIO) | (1ULL << SCREEN_RST_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // 硬件复位（如果使用了RST引脚）
    gpio_set_level(SCREEN_RST_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(SCREEN_RST_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // SCREEN初始化命令序列:cite[9]
    write_cmd(SCREEN_SETDISPLAYOFF);       // 关闭显示
    write_cmd(SCREEN_SETDISPLAYCLOCKDIV);  // 设置时钟分频
    write_cmd(0x80);                       // 建议比率
    write_cmd(SCREEN_SETMULTIPLEX);        // 设置多路复用
    write_cmd(0x3F);                       // 64-1 (对于128x64屏幕)
    write_cmd(SCREEN_SETDISPLAYOFFSET);    // 设置显示偏移
    write_cmd(0x00);                       // 无偏移
    write_cmd(SCREEN_SETSTARTLINE | 0x00); // 设置起始行
    write_cmd(SCREEN_CHARGEPUMP);          // 电荷泵设置
    write_cmd(0x14);                       // 启用电荷泵（内部VCC）
    write_cmd(SCREEN_MEMORYMODE);          // 内存模式
    write_cmd(0x00);                       // 水平寻址模式
    write_cmd(SCREEN_SEGREMAP | 0x01);     // 段重映射（左右翻转，可调整）
    write_cmd(SCREEN_COMSCANDEC);          // 扫描方向（上下翻转，可调整）
    write_cmd(SCREEN_SETCOMPINS);          // 设置COM引脚
    write_cmd(0x12);                       // 对于128x64屏幕的序列
    write_cmd(SCREEN_SETCONTRAST);         // 设置对比度
    write_cmd(0xCF);                       // 对比度值
    write_cmd(SCREEN_SETPRECHARGE);        // 设置预充电期
    write_cmd(0xF1);                       // 预充电期
    write_cmd(SCREEN_DISPLAYALLON_RESUME); // 全部打开显示恢复
    write_cmd(SCREEN_NORMALDISPLAY);       // 正常显示（非反色）
    write_cmd(SCREEN_SETDISPLAYON);        // 打开显示

    clear_all();

    update();
}

void Screen::write_cmd(uint8_t cmd)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    gpio_set_level(SCREEN_DC_GPIO, 0); // DC引脚拉低表示命令
    t.length = 8;
    t.tx_buffer = &cmd;
    spi_device_polling_transmit(spi, &t);
}
void Screen::write_data(const uint8_t *data, size_t len)
{
    spi_transaction_t t;
    if (len == 0)
        return;
    memset(&t, 0, sizeof(t));
    gpio_set_level(SCREEN_DC_GPIO, 1); // DC引脚拉高表示数据
    t.length = len * 8;
    t.tx_buffer = data;
    spi_device_polling_transmit(spi, &t);
}
void Screen::draw_rectangle_px(int x, int y, int width, int height, int color)
{
    for (int i = x; i < x + width; i++)
    {
        for (int j = y; j < y + height; j++)
        {
            if (i >= 0 && i < SCREEN_WIDTH && j >= 0 && j < SCREEN_HEIGHT)
            {
                if (color)
                {
                    buffer[i + (j / 8) * SCREEN_WIDTH] |= (1 << (j % 8));
                }
                else
                {
                    buffer[i + (j / 8) * SCREEN_WIDTH] &= ~(1 << (j % 8));
                }
            }
        }
    }
}
void Screen::draw_word_px(const uint8_t *map, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    for (size_t i = x; i < x + width; ++i)
    {
        for (size_t j = y; j < y + height; j++)
        {
            if (i < SCREEN_WIDTH && j < SCREEN_HEIGHT)
            {
                buffer[i + j / 8 * SCREEN_WIDTH] |= map[(i - x) + (j - y) / 8 * width];
            }
        }
    }
}
void Screen::draw_word_word(const uint8_t *map, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    draw_word_px(map, x * 8, y * 16, width * 8, height * 16);
}

void Screen::invert_px(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    for (size_t i = x; i < x + width; ++i)
    {
        for (size_t j = y; j < y + height; j++)
        {
            if (i < SCREEN_WIDTH && j < SCREEN_HEIGHT)
            {
                buffer[i + j / 8 * SCREEN_WIDTH] ^= (1 << (j % 8));
            }
        }
    }
}
void Screen::invert_word(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    invert_px(x * 8, y * 16, width * 8, height * 16);
}

void Screen::display_init(void)
{
    draw_word_word(word_chu_shi_jie_mian, 0, 0, 8, 1);
    draw_word_word(word_cai_dan, 12, 3, 4, 1);
}
void Screen::display_setting(void)
{
    draw_word_word(word_cai_dan, 0, 0, 4, 1);
    draw_word_word(word_deng_guang, 2, 1, 4, 1);
    draw_word_word(word_yin_yu, 10, 1, 4, 1);
    draw_word_word(word_yin_yue, 2, 2, 4, 1);
    draw_word_word(word_yin_you, 10, 2, 4, 1);
    add_asterisk_word(0, 1);
    draw_word_word(word_que_ren, 0, 3, 4, 1);
    add_slash_word(4, 3);
    draw_word_word(word_tui_chu, 5, 3, 4, 1);
}
void Screen::display_led(void)
{
    draw_word_word(word_deng_guang, 0, 0, 4, 1);
    draw_word_word(word_tui_chu, 0, 3, 4, 1);
}
void Screen::display_buzzer(void)
{
    draw_word_word(word_yin_yu, 0, 0, 4, 1);
    draw_word_word(word_tui_chu, 0, 3, 4, 1);
}
void Screen::display_music_on(void)
{
    draw_word_word(word_zan_ting, 0, 3, 4, 1);
    add_slash_word(4, 3);
    draw_word_word(word_tui_chu, 5, 3, 4, 1);
    draw_word_word(word_chong_lai, 12, 3, 4, 1);
}
void Screen::display_music_pause(void)
{
    draw_word_word(word_ji_xu, 0, 3, 4, 1);
    add_slash_word(4, 3);
    draw_word_word(word_tui_chu, 5, 3, 4, 1);
    draw_word_word(word_chong_lai, 12, 3, 4, 1);
}
void Screen::display_music_start(void)
{
    draw_word_word(word_yin_yue, 0, 0, 4, 1);
    display_music_on();
}
void Screen::display_game(void)
{
    draw_word_word(word_yin_you, 0, 0, 4, 1);
    draw_word_word(word_tui_chu, 0, 3, 4, 1);
}
void Screen::display_game_perfect(void)
{
    add_words("Perfect", 4, 1);
}
void Screen::display_game_good(void)
{
    add_words("Good", 6, 1);
}
void Screen::display_game_bad(void)
{
    add_words("Bad", 6, 1);
}
void Screen::display_game_miss(void)
{
    add_words("Miss", 6, 1);
}
void Screen::display_game_end(uint64_t max_combo)
{
    add_words("Game End", 4, 1);
    add_words("Max Combo is", 0, 2);
    add_num_unsigned(max_combo, 13, 2);
}
void Screen::play_boot_animation(void)
{
    for (int i = 0; i < numof(animation); ++i)
    {
        clear_all();
        memcpy(buffer, animation[i], sizeof(animation[0]));
        update();
        delay_ms(64);
    }
}

void Screen::add_asterisk_word(uint8_t x, uint8_t y)
{
    draw_word_word(word_asterisk, x, y, 2, 1);
}
void Screen::add_sharp_word(uint8_t x, uint8_t y)
{
    draw_word_word(word_sharp, x, y, 2, 1);
}
void Screen::add_connect_word(uint8_t x, uint8_t y)
{
    draw_word_word(word_connect, x, y, 2, 1);
}
void Screen::add_plus_word(uint8_t x, uint8_t y)
{
    draw_word_word(word_plus, x, y, 2, 1);
}
void Screen::add_slash_word(uint8_t x, uint8_t y)
{
    draw_word_word(word_slash, x, y, 2, 1);
}
void Screen::add_char(uint8_t byte, uint8_t x, uint8_t y)
{
    draw_word_word(word_char[char_idx(byte)], x, y, 2, 1);
}
void Screen::add_words(const char *str, uint8_t x, uint8_t y)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i)
    {
        if ((x + i < 16) && ((str[i] >= 'a' && str[i] < 'z') || (str[i] >= 'A' && str[i] < 'Z') || ((str[i] >= '0' && str[i] < '9'))))
        {
            add_char(str[i], x + i, y);
        }
    }
}
void Screen::add_hex(uint8_t num, uint8_t x, uint8_t y)
{
    if ((num >> 4) > 9)
    {
        draw_word_word(word_char[char_idx((num >> 4) - 0xA + 'A')], x, y, 2, 1);
    }
    else
    {
        draw_word_word(word_char[char_idx((num >> 4) + '0')], x, y, 2, 1);
    }
    if ((num & 0xF) > 9)
    {
        draw_word_word(word_char[char_idx((num & 0xF) - 0xA + 'A')], x + 1, y, 2, 1);
    }
    else
    {
        draw_word_word(word_char[char_idx((num & 0xF) + '0')], x + 1, y, 2, 1);
    }
}
void Screen::add_num_unsigned(uint8_t num, uint8_t x, uint8_t y)
{
    int i = 0;
    if (num / 100)
    {
        add_char(num / 100 + '0', x + (i++), y);
    }
    if (num / 10)
    {
        add_char(num / 10 % 10 + '0', x + (i++), y);
    }
    add_char(num % 10 + '0', x + i, y);
}
void Screen::add_num_signed(int8_t num, uint8_t x, uint8_t y)
{
    if (num > 0)
    {
        add_plus_word(x, y);
        add_num_unsigned(num, x + 1, y);
    }
    else if (num < 0)
    {
        add_connect_word(x, y);
        add_num_unsigned(-num, x + 1, y);
    }
    else
    {
        add_num_unsigned(0, x, y);
    }
}
void Screen::add_num(int8_t num, uint8_t x, uint8_t y)
{
    if (num < 0)
    {
        add_connect_word(x, y);
        add_num_unsigned(-num, x + 1, y);
    }
    else
    {
        add_num_unsigned(num, x, y);
    }
}
void Screen::add_pitch(uint8_t pitch, uint8_t x, uint8_t y)
{
    switch (pitch % 12)
    {
    case 0:
    {
        add_char('C', x, y);
        add_char(pitch / 12 + '0', x + 1, y);
        break;
    }
    case 1:
    {
        add_char('C', x, y);
        add_sharp_word(x + 1, y);
        add_char(pitch / 12 + '0', x + 2, y);
        break;
    }
    case 2:
    {
        add_char('D', x, y);
        add_char(pitch / 12 + '0', x + 1, y);
        break;
    }
    case 3:
    {
        add_char('D', x, y);
        add_sharp_word(x + 1, y);
        add_char(pitch / 12 + '0', x + 2, y);
        break;
    }
    case 4:
    {
        add_char('E', x, y);
        add_char(pitch / 12 + '0', x + 1, y);
        break;
    }
    case 5:
    {
        add_char('F', x, y);
        add_char(pitch / 12 + '0', x + 1, y);
        break;
    }
    case 6:
    {
        add_char('F', x, y);
        add_sharp_word(x + 1, y);
        add_char(pitch / 12 + '0', x + 2, y);
        break;
    }
    case 7:
    {
        add_char('G', x, y);
        add_char(pitch / 12 + '0', x + 1, y);
        break;
    }
    case 8:
    {
        add_char('G', x, y);
        add_sharp_word(x + 1, y);
        add_char(pitch / 12 + '0', x + 2, y);
        break;
    }
    case 9:
    {
        add_char('A', x, y);
        add_char(pitch / 12 + '0', x + 1, y);
        break;
    }
    case 10:
    {
        add_char('A', x, y);
        add_sharp_word(x + 1, y);
        add_char(pitch / 12 + '0', x + 2, y);
        break;
    }
    case 11:
    {
        add_char('B', x, y);
        add_char(pitch / 12 + '0', x + 1, y);
        break;
    }
    }
}
void Screen::add_pitch_line(uint8_t pitch, uint8_t x, uint8_t y)
{
    draw_word_word(word_yin_ming, x, y, 4, 1);
    add_pitch(pitch, x + 4, y);
}
void Screen::add_yinyu_line(uint8_t offset, uint8_t x, uint8_t y)
{
    add_pitch(offset - 12, x, y);
    add_connect_word(x + 3, y);
    add_pitch(offset + 23, x + 5, y);
}
void Screen::add_lumin_line(uint8_t lumin, uint8_t x, uint8_t y)
{
    draw_word_word(word_liang_du, x, y, 4, 1);
    add_hex(lumin, x + 4, y);
}

void Screen::clear_px(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    for (size_t i = x; i < x + width; ++i)
    {
        for (size_t j = y; j < y + height; j++)
        {
            if (i < SCREEN_WIDTH && j < SCREEN_HEIGHT)
            {
                buffer[i + j / 8 * SCREEN_WIDTH] = 0;
            }
        }
    }
}
void Screen::clear_all(void)
{
    memset(buffer, 0, sizeof(buffer));
}
void Screen::clear_word(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    clear_px(x * 8, y * 16, width * 8, height * 16);
}
void Screen::clear_main(void)
{
    clear_px(0, 16, 128, 32);
}
void Screen::clear_main_line1(void)
{
    clear_px(0, 16, 128, 16);
}
void Screen::clear_main_line2(void)
{
    clear_px(0, 32, 128, 16);
}
void Screen::clear_head(void)
{
    clear_px(0, 0, 128, 16);
}
void Screen::clear_tail(void)
{
    clear_px(0, 48, 128, 16);
}

void Screen::update(void)
{
    write_cmd(SCREEN_COLUMNADDR);
    write_cmd(0);   // 列起始地址
    write_cmd(127); // 列结束地址
    write_cmd(SCREEN_PAGEADDR);
    write_cmd(0); // 页起始地址
    write_cmd(7); // 页结束地址（对于64像素高度）
    write_data(buffer, sizeof(buffer));
}
