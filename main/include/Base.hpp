#ifndef BASE_HPP
#define BASE_HPP
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include<queue>

/**************************
 *      Basic Function
 **************************/
using std::min;
using std::max;
#define clamp(_value,_min,_max) (min(max(_value,_min),_max))
#define numof(_list) (sizeof(_list) / sizeof(_list[0]))

/**************************
 *      Delay Function
 **************************/
#define delay_ms(_ms) (vTaskDelay(pdMS_TO_TICKS(_ms)))
#define delay_us(_us) (ets_delay_us(_us))

/**************************
 *    SignalQueue Class
 **************************/
template <typename _Tp>
class SignalQueue
{
private:
    std::queue<_Tp> m_signals;

public:
    SignalQueue(void) : m_signals() {}
    SignalQueue(const SignalQueue &) = default;
    ~SignalQueue(void) = default;

public:
    SignalQueue &operator=(const SignalQueue &) = default;

public:
    size_t size(void)
    {
        return m_signals.size();
    }
    void push(const _Tp &_signal)
    {
        m_signals.push(_signal);
    }
    void push(_Tp &&_signal)
    {
        m_signals.push(std::move(_signal));
    }
    void pop(void)
    {
        m_signals.pop();
    }
    template <typename _Func, typename... _Args>
    void handle_one(_Func _func, _Args &&..._args)
    {
        if (m_signals.size())
        {
            _func(m_signals.front(), std::forward<_Tp>(_args)...);
            m_signals.pop();
        }
    }
    template <typename _Func, typename... _Args>
    void handle_all(_Func _func, _Args &&..._args)
    {
        while (m_signals.size())
        {
            _func(m_signals.front(), std::forward<_Tp>(_args)...);
            m_signals.pop();
        }
    }
    void clear(void){
        while(m_signals.size()){
            m_signals.pop();
        }
    }

public:
    _Tp &get(void)
    {
        return m_signals.front();
    }
    const _Tp &get(void) const
    {
        return m_signals.front();
    }
};

/**************************
 *        pin gpio
 **************************/
//touch
#define TOUCH_SDO_GPIO      GPIO_NUM_26
#define TOUCH_SCL_GPIO      GPIO_NUM_27
//buzzer
#define BUZZER_GPIO         GPIO_NUM_25
//led
#define LED_GPIO            GPIO_NUM_2
//screen
#define SCREEN_CS_GPIO      GPIO_NUM_19
#define SCREEN_DC_GPIO      GPIO_NUM_18
#define SCREEN_RST_GPIO     GPIO_NUM_5
#define SCREEN_MOSI_GPIO    GPIO_NUM_17
#define SCREEN_CLK_GPIO     GPIO_NUM_4

#endif