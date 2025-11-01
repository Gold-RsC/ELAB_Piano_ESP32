#include "WS2812B.hpp"
#include "TTP229.hpp"
#include "Buzzer.hpp"
#include "OLED.hpp"

/**************************
 *          Model
 **************************/
Buzzer buzzer;
Touch touch;
LED led;
Screen screen;

/**************************
 *       Task Delay
 **************************/
#define MAIN_TASK_DELAY 10
#define BUZZER_TASK_DELAY 20

/**************************
 *       Hit SQueue
 **************************/
SignalQueue<uint8_t> game_hit_queue;
uint8_t hit_list[] = {
    KEY_NOTE0_IDX, KEY_NOTE7_IDX, KEY_NOTE4_IDX, KEY_NOTE11_IDX,
    KEY_NOTE4_IDX, KEY_NOTE11_IDX, KEY_NOTE7_IDX, KEY_NOTE0_IDX,
    KEY_NOTE4_IDX, KEY_NOTE7_IDX, KEY_NOTE11_IDX, KEY_NOTE0_IDX,
    KEY_NOTE11_IDX, KEY_NOTE4_IDX, KEY_NOTE7_IDX, KEY_NOTE0_IDX,
    KEY_NOTE7_IDX, KEY_NOTE11_IDX, KEY_NOTE0_IDX, KEY_NOTE11_IDX,
    KEY_NOTE4_IDX, KEY_NOTE7_IDX, KEY_NOTE0_IDX, KEY_NOTE11_IDX,
    KEY_NOTE4_IDX, KEY_NOTE7_IDX, KEY_NOTE4_IDX, KEY_NOTE11_IDX,
    KEY_NOTE0_IDX, KEY_NOTE7_IDX, KEY_NOTE0_IDX, KEY_NOTE7_IDX,
    KEY_NOTE11_IDX, KEY_NOTE4_IDX, KEY_NOTE11_IDX, KEY_NOTE7_IDX,
    KEY_NOTE0_IDX, KEY_NOTE7_IDX, KEY_NOTE4_IDX, KEY_NOTE11_IDX,
    KEY_NOTE4_IDX, KEY_NOTE7_IDX};
#define HIT_LIST_NUM numof(hit_list)
uint8_t hit_idx = 0;

/**************************
 *     GameTask Handle
 **************************/
TaskHandle_t game_handle=NULL;
TaskHandle_t music_handle=NULL;
TaskHandle_t animation_screen_handle=NULL,animation_music_handle=NULL;
/**************************
 *      Init Function
 **************************/
void init(void)
{
    touch.init();
    buzzer.init();
    led.init();
    screen.init();
}

/**************************
 *      Double Click
 **************************/
#define DBCL_DTMS 200
#define INIT_DBCL_TIMER(letter) (touch##letter##_timer = 0)
#define FINISH_DBCL_TIMER(letter) (touch##letter##_timer = -1)
#define UPDATE_DBCL_TIMER(letter) (touch##letter##_timer = DBCL_DTMS / MAIN_TASK_DELAY)

/**************************
 *      Trigger Mode
 **************************/
#define continuous_trigger() if (true)
#define single_trigger() if (current_keys != previous_keys)

/**************************
 *     PlaySound Task
 **************************/
char tmp_task_name[16][7] = {
    "sound0", "sound1", "sound2", "sound3", "sound4", "sound5", "sound6", "sound7", "sound8", "sound9", "soundA", "soundB", "soundC", "soundD", "soundE", "soundF"};
uint8_t tmp_task_name_idx = 0;
#define TMP_TASK_NAME_NUM 16
void play_sound_next_task(void *)
{
    buzzer.play_music_next();
    vTaskDelete(NULL);
}
void play_sound_previous_task(void *)
{
    buzzer.play_music_previous();
    vTaskDelete(NULL);
}
#define play_sound_next() (xTaskCreate(play_sound_next_task, tmp_task_name[(++tmp_task_name_idx) %= TMP_TASK_NAME_NUM], 4096, NULL, 2, NULL))
#define play_sound_previous() (xTaskCreate(play_sound_previous_task, tmp_task_name[(++tmp_task_name_idx) %= TMP_TASK_NAME_NUM], 4096, NULL, 2, NULL))

/**************************
 *       Main State
 **************************/
enum main_state_t
{
    main_state_animation,
    main_state_init,
    main_state_setting,
    main_state_music,
    main_state_buzzer,
    main_state_led,
    main_state_game
};
main_state_t main_state = main_state_animation;
uint8_t base_offset = 60;

/**************************
 *    Switch Interface
 **************************/
uint8_t setting_idx = 0;
enum
{
    setting_led = 0,
    setting_buzzer = 1,
    setting_music = 2,
    setting_game = 3
};
#define SETTING_NUM 4
void music_task(void*);
void game_task(void*);
void setting_up(void)
{
    play_sound_next();
    (++setting_idx) %= SETTING_NUM;
    screen.clear_word(0, 1, 2, 2);
    screen.clear_word(8, 1, 2, 2);
    switch (setting_idx)
    {
    case setting_led:
    {
        screen.add_asterisk_word(0, 1);
        break;
    }
    case setting_buzzer:
    {
        screen.add_asterisk_word(8, 1);
        break;
    }
    case setting_music:
    {
        screen.add_asterisk_word(0, 2);
        break;
    }
    case setting_game:
    {
        screen.add_asterisk_word(8, 2);
        break;
    }
    default:
    {
    }
    }
    screen.update();
}
void setting_down(void)
{
    play_sound_previous();
    setting_idx = (setting_idx + SETTING_NUM - 1) % SETTING_NUM;
    screen.clear_word(0, 1, 2, 2);
    screen.clear_word(8, 1, 2, 2);
    switch (setting_idx)
    {
    case setting_led:
    {
        screen.add_asterisk_word(0, 1);
        break;
    }
    case setting_buzzer:
    {
        screen.add_asterisk_word(8, 1);
        break;
    }
    case setting_music:
    {
        screen.add_asterisk_word(0, 2);
        break;
    }
    case setting_game:
    {
        screen.add_asterisk_word(8, 2);
        break;
    }
    default:
    {
    }
    }
    screen.update();
}
void change_setting_to_init(void)
{
    main_state = main_state_init;
    led.state = led_state_wave;
    buzzer.state.mode = buzzer_mode_init;
    buzzer.buzzer_off();
    screen.clear_all();
    screen.display_init();
    screen.update();
}
void change_init_to_setting(void)
{
    main_state = main_state_setting;
    led.state = led_state_choose;
    buzzer.state.mode = buzzer_mode_choose;
    setting_idx = 0;
    buzzer.buzzer_off();
    screen.clear_all();
    screen.display_setting();
    screen.update();
}
void change_led_to_setting(void)
{
    main_state = main_state_setting;
    led.state = led_state_choose;
    buzzer.state.mode = buzzer_mode_choose;
    setting_idx = 0;
    buzzer.buzzer_off();
    screen.clear_all();
    screen.display_setting();
    screen.update();
}
void change_buzzer_to_setting(void)
{
    main_state = main_state_setting;
    led.state = led_state_choose;
    buzzer.state.mode = buzzer_mode_choose;
    setting_idx = 0;
    buzzer.buzzer_off();
    screen.clear_all();
    screen.display_setting();
    screen.update();
}
void change_music_to_setting(void)
{
    main_state = main_state_setting;
    led.state = led_state_choose;
    buzzer.state.mode = buzzer_mode_choose;
    vTaskDelete(music_handle);
    music_handle=NULL;
    setting_idx = 0;
    buzzer.buzzer_off();
    screen.clear_all();
    screen.display_setting();
    screen.update();
}
void change_game_to_setting(void)
{
    game_hit_queue.clear();
    vTaskDelete(game_handle);
    game_handle=NULL;
    buzzer.buzzer_off();
    main_state = main_state_setting;
    led.state = led_state_choose;
    buzzer.state.mode = buzzer_mode_choose;
    setting_idx = 0;
    screen.clear_all();
    screen.display_setting();
    screen.update();
}
void changeto_music(void)
{
    main_state = main_state_music;
    led.state = led_state_wave;
    buzzer.state.mode = buzzer_mode_music_on;
    xTaskCreate(music_task, "music", 2500, NULL, 3, &music_handle);
    buzzer.buzzer_off();
    screen.clear_all();
    screen.display_music_start();
    screen.update();
}
void changeto_buzzer(void)
{
    main_state = main_state_buzzer;
    led.state = led_state_choose;
    buzzer.state.mode = buzzer_mode_choose;
    buzzer.buzzer_off();
    screen.clear_all();
    screen.display_buzzer();
    screen.add_yinyu_line(base_offset, 3, 1);
    screen.update();
}
void changeto_game(void)
{
    main_state = main_state_game;
    led.state = led_state_game;
    buzzer.state.mode = buzzer_mode_game;
    buzzer.buzzer_off();
    game_hit_queue.clear();
    xTaskCreate(game_task, "game", 2500, NULL, 3, &game_handle);
    hit_idx = 0;
    screen.clear_all();
    screen.display_game();
    screen.update();
}
void changeto_led(void)
{
    main_state = main_state_led;
    led.state = led_state_rainbow;
    buzzer.state.mode = buzzer_mode_choose;
    buzzer.buzzer_off();
    screen.clear_all();
    screen.display_led();
    screen.add_lumin_line(led.luminosity, 5, 1);
    screen.update();
}
void change_animation_to_init(void){
    main_state = main_state_init;
    led.state = led_state_wave;
    buzzer.state.mode = buzzer_mode_init;
    buzzer.buzzer_off();
    screen.clear_all();
    screen.display_init();
    screen.update();
}
/**************************
 *      Task Function
 **************************/
void touch_task(void *)
{
    while (1)
    {
        touch.read();
    }
}
void music_task(void *)
{
    buzzer.play_music_1();
    screen.add_words("END",6,1);
    music_handle=NULL;
    vTaskDelete(NULL);
}
void led_task(void *)
{
    while (1)
    {
        if (led.state == led_state_wave)
        {
            led.close_all();
            led.set_wave_zero();
        }
        while (led.state == led_state_wave)
        {
            led.run_wave();
            delay_ms(50);
        }
        if (led.state == led_state_rainbow)
        {
            led.close_all();
        }
        for (int offset = 0; led.state == led_state_rainbow; (++offset) %= numof(colorCycle_list))
        {

            led.run_rainbow(offset);
            delay_ms(100);
        }
        if (led.state == led_state_choose)
        {
            led.close_all();
            led.set_wave_zero();
        }
        bool flag=1;
        for (int offset = 0; led.state == led_state_choose; (++offset) %= numof(colorCycle_list))
        {
            led.run_choose(offset,flag);
            delay_ms(100);
        }
        if (led.state == led_state_game)
        {
            led.close_all();
        }
        while (led.state == led_state_game)
        {

            delay_ms(100);
        }
    }
}
void buzzer_task(void *)
{
    for (uint64_t i=0;;)
    {
        notes.handle_all([&](uint8_t pitch)
                     {
            if(pitch&0x80){
                if((notes.size()==1)&&buzzer.is_state_on()){
                    buzzer.buzzer_off();
                    i=0;
                }
            }
            else{
                buzzer.set_pitch(pitch);
                if(buzzer.is_state_off()){
                    buzzer.buzzer_on();
                }
                if(buzzer.is_mode_music_on() || buzzer.is_mode_music_pause()){
                    led.set_wave_center(pitch%12);
                }
            } 
        });
        if(buzzer.is_state_on()){
            if(i>200){
                buzzer.buzzer_off();
                i=0;
            }
            ++i;
        }
        else{
            i=0;
        }
        delay_ms(BUZZER_TASK_DELAY);
    }
}
void game_task(void *)
{
    uint64_t combo = 0;
    uint64_t max_combo = 0;
    delay_ms(500);
    for (size_t i = 0; i < numof(music_list_game); ++i)
    {
        if (!(music_list_game[i][1] & 0x80))
        {
            led.set_one(keyNum_idx_map[hit_list[hit_idx]], 0xFF, 0xFF, 0xFF);
            led.update();
            while (1)
            {
                if (game_hit_queue.size())
                {
                    if (game_hit_queue.get() == hit_list[hit_idx])
                    {
                        game_hit_queue.pop();
                        led.clear_one(keyNum_idx_map[hit_list[hit_idx]]);
                        led.update();
                        screen.clear_main();
                        screen.add_num_unsigned(++combo, 13, 2);
                        screen.display_game_good();
                        screen.update();
                        notes.push(music_list_game[i][1]);
                        (++hit_idx) %= HIT_LIST_NUM;
                        max_combo=max(combo,max_combo);
                        break;
                    }
                    else
                    {
                        combo = 0;
                        screen.clear_main();
                        screen.add_num_unsigned(combo, 13, 2);
                        screen.display_game_bad();
                        screen.update();
                        game_hit_queue.pop();
                    }
                }
                delay_ms(10);
            }
        }
    }
    screen.clear_main();
    screen.display_game_end(max_combo);
    screen.update();
    while (1)
    {
        delay_ms(100);
    }
}
void animation_screen_task(void*){
    screen.clear_all();
    screen.play_boot_animation();
    
    if(animation_music_handle){
        vTaskDelete(animation_music_handle);
        animation_music_handle=NULL;
    }
    change_animation_to_init();
    animation_screen_handle=NULL;
    vTaskDelete(NULL);
}
void animation_music_task(void*){
    buzzer.play_music_animation();
    if(animation_screen_handle){
        vTaskDelete(animation_screen_handle);
        animation_screen_handle=NULL;
    }
    change_animation_to_init();
    animation_music_handle=NULL;
    vTaskDelete(NULL);
}
void main_task(void *)
{
    uint16_t current_keys = 0xFFFF;
    uint16_t previous_keys = 0xFFFF;
    int8_t touchA_timer = 0, touchB_timer = 0;
    
    while (1)
    {
        touch.keys.handle_one([&](uint16_t _k)
                          { current_keys = _k; });
        
        single_trigger()
        {
            uint16_t changing = current_keys ^ previous_keys;
            uint16_t pressing = changing & (~current_keys);
            uint16_t releasing = changing & current_keys;

            switch (main_state)
            {
            case main_state_animation:{
                change_animation_to_init();
                if(animation_screen_handle){
                    vTaskDelete(animation_screen_handle);
                    animation_screen_handle=NULL;
                }
                if(animation_music_handle){
                    vTaskDelete(animation_music_handle);
                    animation_music_handle=NULL;
                }
                
                break;
            }
            case main_state_init:
            {
                uint8_t offset = base_offset;
                if (PRESSING_LETTER(E))
                {
                    screen.clear_word(0, 2, 3, 1);
                    screen.add_num_signed(+12, 0, 2);
                    screen.update();
                }
                if (PRESSING_LETTER(L))
                {
                    screen.clear_word(13, 2, 3, 1);
                    screen.add_num_signed(-12, 13, 2);
                    screen.update();
                }
                if (PRESSED_LETTER(E))
                {
                    offset = clamp(offset + 12, 24, 96);
                }
                if (PRESSED_LETTER(L))
                {
                    offset = clamp(offset - 12, 24, 96);
                }
                if (RELEASING_LETTER(E))
                {
                    screen.clear_word(0, 2, 3, 1);
                    screen.update();
                }
                if (RELEASING_LETTER(L))
                {
                    screen.clear_word(13, 2, 3, 1);
                    screen.update();
                }
                for (int i = 0; i < 16; ++i)
                {
                    if (IS_NOTE_IDX(i))
                    {
                        if (PRESSING_IDX(i))
                        {
                            notes.push((uint8_t)keyNum_idx_map[i] + offset);
                            screen.clear_main_line1();
                            screen.add_pitch_line((uint8_t)keyNum_idx_map[i] + offset, 5, 1);
                            screen.update();
                        }
                        else if (RELEASING_LAST_IN_NOTES_IDX(i))
                        {
                            notes.push(0x80);
                            screen.clear_main_line1();
                            screen.update();
                        }
                    }
                }
                if (PRESSING_LETTER(B))
                {
                    if (touchB_timer > 0)
                    {
                        FINISH_DBCL_TIMER(B);
                    }
                    else
                    {
                        screen.invert_word(12, 3, 4, 1);
                        screen.update();
                    }
                }
                if (RELEASING_LETTER(B))
                {
                    if (!touchB_timer)
                    {
                        UPDATE_DBCL_TIMER(B);
                    }
                    else if (touchB_timer == -1)
                    {
                        INIT_DBCL_TIMER(B);
                        change_init_to_setting();
                    }
                }

                break;
            }
            case main_state_setting:
            {
                if (PRESSING_LETTER(A))
                {
                    if (touchA_timer > 0)
                    {
                        FINISH_DBCL_TIMER(A);
                        screen.invert_word(0, 3, 4, 1);
                        screen.invert_word(5, 3, 4, 1);
                        screen.update();
                    }
                    else
                    {
                        screen.invert_word(0, 3, 4, 1);
                        screen.update();
                    }
                }
                if (RELEASING_LETTER(A))
                {
                    if (!touchA_timer)
                    {
                        UPDATE_DBCL_TIMER(A);
                    }
                    else if (touchA_timer == -1)
                    {
                        INIT_DBCL_TIMER(A);
                        change_setting_to_init();
                    }
                }
                if (PRESSING_NOTE(11))
                {
                    
                    setting_up();
                }
                else if (PRESSING_NOTE(0))
                {
                    
                    setting_down();
                }
                if(RELEASING_NOTE(11)){
                    led.set_wave_center(11);
                }
                else if(RELEASING_NOTE(0)){
                    
                    led.set_wave_center(0);
                }

                break;
            }
            case main_state_buzzer:
            {
                if (PRESSING_LETTER(A))
                {
                    if (touchA_timer > 0)
                    {
                        FINISH_DBCL_TIMER(A);
                    }
                    else
                    {
                        screen.invert_word(0, 3, 4, 1);
                        screen.update();
                    }
                }
                if (RELEASING_LETTER(A))
                {
                    if (!touchA_timer)
                    {
                        UPDATE_DBCL_TIMER(A);
                    }
                    else if (touchA_timer == -1)
                    {
                        INIT_DBCL_TIMER(A);
                        change_buzzer_to_setting();
                    }
                }
                if (PRESSING_NOTE(11))
                {
                    
                    play_sound_next();
                    base_offset = clamp(base_offset + 12, 36, 72);
                    screen.clear_main_line1();
                    screen.add_yinyu_line(base_offset, 3, 1);
                    screen.update();
                }
                else if (PRESSING_NOTE(0))
                {
                    play_sound_previous();
                    base_offset = clamp(base_offset - 12, 36, 72);
                    screen.clear_main_line1();
                    screen.add_yinyu_line(base_offset, 3, 1);
                    screen.update();
                }
                if(RELEASING_NOTE(11)){
                    led.set_wave_center(11);
                }
                else if(RELEASING_NOTE(0)){
                    led.set_wave_center(0);
                }
                for (int i = 0; i < 16; ++i)
                {
                    if (IS_NOTE_IDX(i) && i != KEY_NOTE11_IDX && i != KEY_NOTE0_IDX)
                    {
                        if (PRESSING_IDX(i))
                        {
                            notes.push(base_offset);
                            screen.clear_main_line2();
                            screen.add_pitch_line(base_offset, 5, 2);
                            screen.update();
                        }
                        else if (RELEASING_LAST_IN_NOTES_IDX(i))
                        {
                            notes.push(0x80);
                            screen.clear_main_line2();
                            screen.update();
                        }
                    }
                }
                break;
            }
            case main_state_led:
            {
                if (PRESSING_LETTER(A))
                {
                    if (touchA_timer > 0)
                    {
                        FINISH_DBCL_TIMER(A);
                    }
                    else
                    {
                        screen.invert_word(0, 3, 4, 1);
                        screen.update();
                    }
                }
                if (RELEASING_LETTER(A))
                {
                    if (!touchA_timer)
                    {
                        UPDATE_DBCL_TIMER(A);
                    }
                    else if (touchA_timer == -1)
                    {
                        INIT_DBCL_TIMER(A);
                        change_led_to_setting();
                    }
                }

                for (int i = 0; i < 16; ++i)
                {
                    if (IS_NOTE_IDX(i))
                    {
                        if (PRESSING_IDX(i))
                        {
                            if (i == KEY_NOTE11_IDX)
                            {
                                led.luminosity = 0xFF;
                            }
                            else
                            {
                                led.luminosity = keyNum_idx_map[i] * 23;
                            }
                            notes.push((uint8_t)keyNum_idx_map[i] + 60);
                            screen.clear_main();
                            screen.add_lumin_line(led.luminosity, 5, 1);
                            screen.update();
                        }
                        else if (RELEASING_LAST_IN_NOTES_IDX(i))
                        {
                            notes.push(0x80);
                        }
                    }
                }
                break;
            }
            case main_state_music:
            {
                if (PRESSING_LETTER(B))
                {
                    screen.invert_word(12, 3, 4, 1);
                    screen.update();
                }
                if (RELEASING_LETTER(B))
                {
                    screen.invert_word(12, 3, 4, 1);
                    screen.update();
                    buzzer.state.reset = 1;
                }
                if (PRESSING_LETTER(A))
                {
                    if (touchA_timer > 0)
                    {
                        FINISH_DBCL_TIMER(A);
                        screen.invert_word(0, 3, 4, 1);
                        screen.invert_word(5, 3, 4, 1);
                        screen.update();
                    }
                    else
                    {
                        screen.invert_word(0, 3, 4, 1);
                        screen.update();
                    }
                }
                if (RELEASING_LETTER(A))
                {
                    if (!touchA_timer)
                    {
                        UPDATE_DBCL_TIMER(A);
                    }
                    else if (touchA_timer == -1)
                    {
                        INIT_DBCL_TIMER(A);
                        change_music_to_setting();
                    }
                }
                break;
            }
            case main_state_game:
            {
                for (int i = 0; i < 16; ++i)
                {
                    if (PRESSING_IDX(i) && IS_GAMEKEY_IDX(i))
                    {
                        game_hit_queue.push(i);
                    }
                    if (RELEASING_IDX(i) && IS_GAMEKEY_IDX(i))
                    {
                        notes.push(0x80);
                    }
                }

                if (PRESSING_LETTER(A))
                {
                    if (touchA_timer > 0)
                    {
                        FINISH_DBCL_TIMER(A);
                    }
                    else
                    {
                        screen.invert_word(0, 3, 4, 1);
                        screen.update();
                    }
                }
                if (RELEASING_LETTER(A))
                {
                    if (!touchA_timer)
                    {
                        UPDATE_DBCL_TIMER(A);
                    }
                    else if (touchA_timer == -1)
                    {
                        INIT_DBCL_TIMER(A);

                        change_game_to_setting();
                    }
                }
                break;
            }
            default:
            {
            }
            }

            previous_keys = current_keys;
        }

        continuous_trigger()
        {
            switch (main_state)
            {
            case main_state_animation:{

                break;
            }
            case main_state_init:
            {
                for (int i = 0; i < 16; ++i)
                {
                    if (IS_NOTE_IDX(i) && PRESSED_IDX(i))
                    {
                        led.set_wave_center(keyNum_idx_map[i]);
                    }
                }
                break;
            }
            case main_state_setting:
            {

                break;
            }
            case main_state_led:
            {
                break;
            }
            case main_state_buzzer:
            {
                for (int i = 0; i < 16; ++i)
                {
                    if (IS_NOTE_IDX(i) && PRESSED_IDX(i))
                    {
                        led.set_wave_center(keyNum_idx_map[i]);
                    }
                }
                break;
            }
            case main_state_music:
            {
                break;
            }
            case main_state_game:
            {
                break;
            }
            default:
            {
            }
            }
            if (touchA_timer > 0)
            {
                --touchA_timer;
                if (!touchA_timer)
                {
                    switch (main_state)
                    {
                    case main_state_animation:{
                        break;
                    }
                    case main_state_init:
                    {

                        break;
                    }
                    case main_state_music:
                    {
                        if (buzzer.is_mode_music_on())
                        {
                            buzzer.state.mode = buzzer_mode_music_pause;
                            screen.clear_tail();
                            screen.display_music_pause();
                            screen.update();
                        }
                        else if (buzzer.is_mode_music_pause())
                        {
                            buzzer.state.mode = buzzer_mode_music_on;
                            screen.clear_tail();
                            screen.display_music_on();
                            screen.update();
                        }
                        break;
                    }
                    case main_state_buzzer:
                    {
                        screen.invert_word(0, 3, 4, 1);
                        screen.update();
                        break;
                    }
                    case main_state_setting:
                    {
                        switch (setting_idx)
                        {
                        case setting_music:
                        {
                            changeto_music();
                            break;
                        }
                        case setting_buzzer:
                        {
                            changeto_buzzer();
                            break;
                        }
                        case setting_led:
                        {
                            changeto_led();
                            break;
                        }
                        case setting_game:
                        {
                            changeto_game();
                            break;
                        }
                        default:
                        {
                        }
                        }
                        setting_idx = 0;
                        break;
                    }
                    case main_state_led:
                    {
                        screen.invert_word(0, 3, 4, 1);
                        screen.update();
                        break;
                    }
                    case main_state_game:
                    {

                        screen.invert_word(0, 3, 4, 1);
                        screen.update();
                        break;
                    }
                    default:
                    {
                    }
                    }
                }
            }
            if (touchB_timer > 0)
            {
                --touchB_timer;
                if (!touchB_timer && main_state == main_state_init)
                {
                    screen.invert_word(12, 3, 4, 1);
                    screen.update();
                }
            }
        }

        delay_ms(MAIN_TASK_DELAY);
    }
}

/**************************
 *      Main Function
 **************************/
extern "C" void app_main(void)
{
    init();  
    xTaskCreate(touch_task, "touch", 2500, NULL, 7, &touch_task_handle);
    xTaskCreate(main_task, "main", 2500, NULL, 6, NULL);
    delay_ms(1000);
    xTaskCreate(led_task, "led", 2500, NULL, 5, NULL);
    xTaskCreate(buzzer_task, "buzzer", 3000, NULL, 4, NULL);
    xTaskCreate(animation_screen_task,"ani_screen", 2500, NULL, 3, &animation_screen_handle);
    xTaskCreate(animation_music_task,"ani_music", 2500, NULL, 3, &animation_music_handle);
    while (1)
    {
        delay_ms(100);
    }
}