#ifndef TOUCH_HPP
#define TOUCH_HPP
#include "Base.hpp"

/**************************
 *     KeyNum[Idx] Map
 **************************/
inline int8_t keyNum_idx_map[16] = {
    3,
    2,
    1,
    0,
    -1,
    -2,
    -3,
    -4,
    8,
    9,
    10,
    11,
    7,
    6,
    5,
    4};

/**************************
 *      Idx Key Map
 **************************/
#define KEY_E_IDX 4
#define KEY_L_IDX 5
#define KEY_A_IDX 6
#define KEY_B_IDX 7
#define KEY_NOTE0_IDX 3
#define KEY_NOTE1_IDX 2
#define KEY_NOTE2_IDX 1
#define KEY_NOTE3_IDX 0
#define KEY_NOTE4_IDX 15
#define KEY_NOTE5_IDX 14
#define KEY_NOTE6_IDX 13
#define KEY_NOTE7_IDX 12
#define KEY_NOTE8_IDX 8
#define KEY_NOTE9_IDX 9
#define KEY_NOTE10_IDX 10
#define KEY_NOTE11_IDX 11

/**************************
 *      Judgement MD
 **************************/
// 是否是目标下标
#define IS_LETTER_IDX(idx) ((1 << idx) & 0x00F0)
#define IS_NOTE_IDX(idx) ((1 << idx) & 0xFF0F)
#define IS_GAMEKEY_IDX(idx) (i == KEY_NOTE0_IDX || i == KEY_NOTE4_IDX || i == KEY_NOTE7_IDX || i == KEY_NOTE11_IDX)
// 是否已按下/释放
#define PRESSED_IDX(idx) ((~current_keys) & (1 << idx))
#define PRESSED_LETTER(name) PRESSED_IDX(KEY_##name##_IDX)
#define PRESSED_NOTE(num) PRESSED_IDX(KEY_##num##_IDX)
#define RELEASED_IDX(idx) ((current_keys) & (1 << idx))
#define RELEASED_LETTER(name) RELEASED_IDX(KEY_##name##_IDX)
#define RELEASED_NOTE(num) RELEASED_IDX(KEY_##num##_IDX)
// 是否正在按下/释放
#define PRESSING_IDX(idx) (pressing & (1 << idx))
#define PRESSING_LETTER(name) PRESSING_IDX(KEY_##name##_IDX)
#define PRESSING_NOTE(num) PRESSING_IDX(KEY_NOTE##num##_IDX)
#define RELEASING_IDX(idx) (releasing & (1 << idx))
#define RELEASING_LETTER(name) RELEASING_IDX(KEY_##name##_IDX)
#define RELEASING_NOTE(num) RELEASING_IDX(KEY_NOTE##num##_IDX)
// 音符中唯一按下
#define PRESSING_ONLY_IN_NOTES_IDX(idx) ((pressing & (1 << idx)) && ((previous_keys | 0x00F0) == 0xFFFF))
// 字母中唯一按下
#define PRESSING_ONLY_IN_LETTERS_IDX(idx) ((pressing & (1 << idx)) && ((previous_keys | 0xFF0F) == 0xFFFF))
// 音符中最后释放
#define RELEASING_LAST_IN_NOTES_IDX(idx) ((releasing & (1 << idx)) && ((current_keys | 0x00F0) == 0xFFFF))
// 字母中最后释放
#define RELEASING_LAST_IN_LETTERS_IDX(idx) ((releasing & (1 << idx)) && ((current_keys | 0xFF0F) == 0xFFFF))

inline TaskHandle_t touch_task_handle = NULL;
void IRAM_ATTR touch_isr_handler(void *);

/***************************
 *      Touch Class
 **************************/
class Touch
{
public:
    SignalQueue<uint16_t> keys;

private:
    uint16_t previous_keys;

public:
    Touch(void) : previous_keys(0xFFFF) {}

public:
    void init(void);

    void read(void);
};
#endif
