#pragma once

#include <stdio.h>
#include <stdint.h>
#include <functional>
#include "pico/stdlib.h"

class PIDHandler {
public:
    enum AntiWindupType
    {
        //Remember to update _set_anti_windup if new AW is added
        NONE,
        CLAMP
    };

    using Type = float;
    using InputHook  = std::function<Type()>;
    using TimeHook  = std::function<uint64_t()>;
    using OutputHook = std::function<void(Type)>;

    struct Gains { float kp, ki, kd; };

    PIDHandler();
    PIDHandler(Gains gains, const TimeHook time_us_hook, const InputHook state_hook);

    float tick();
    Gains get_gains();
    void set_gains(Gains gains);
    void set_target(Type target);
    int set_anti_windup(AntiWindupType anti_windup_type);
    void set_windup_clamp(Type min, Type max);
    void set_output_limits(Type min, Type max);

private:
    bool initialized = false;
    TimeHook _get_time;
    InputHook _get_state;

    //Last state
    uint64_t _last_time_us;
    Type _last_state;

    //Current state
    Gains _gains;
    Type _integral;
    float _target;

    //Windup values
    AntiWindupType _aw_type;
    bool _is_clamp_set;
    Type _aw_clamp_min;
    Type _aw_clamp_max;

    

    Type _clamp_float(Type value);
};