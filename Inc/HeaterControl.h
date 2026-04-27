#pragma once
#include "pico/stdlib.h"
#include "pico/time.h"

class HeaterControl {
public:
    HeaterControl(uint pin, float freq = 0.6f);
    void init();
    void set_duty(float duty);
    float get_duty();

private:
    uint _pin;
    float _duty;
    uint32_t _tick        = 0;
    uint32_t _ticks_on    = 0;
    repeating_timer_t _timer;

    static constexpr uint32_t TICKS = 100;
    static bool _callback(repeating_timer_t *rt);
};