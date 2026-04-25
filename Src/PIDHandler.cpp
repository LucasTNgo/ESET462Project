#include "PIDHandler.h"

PIDHandler::PIDHandler(Gains gains, TimeHook time_hook, InputHook state_hook)
    : _gains(gains),
      _get_time(time_hook),
      _get_state(state_hook),
      _target(0),
      _integral(0),
      _is_clamp_set(false),
      _aw_type(AntiWindupType::NONE)
{
    _last_time_us = _get_time();
    _last_state = _get_state();
}

PIDHandler::Type PIDHandler::tick()
{
    uint64_t now     = _get_time();
    Type state   = _get_state();

    Type dt      = (now - _last_time_us) / 1000000.0f; // microseconds to seconds
    if (dt <= 0.0001f){ dt = 0.0001f; }

    Type error   = _target - state;

    Type p = _gains.kp * error;

    _integral   += error * dt;
    if(_aw_type == AntiWindupType::CLAMP) { _integral = _clamp_float(_integral); }

    Type i       = _gains.ki * _integral;

    Type d       = _gains.kd * (-(state - _last_state) / dt);

    _last_time_us  = now;
    _last_state = state;

    return p + i + d;
}


PIDHandler::Gains PIDHandler::get_gains() { return _gains; }
void PIDHandler::set_gains(Gains gains) { _gains = gains; }
void PIDHandler::set_target(Type target) { _target = target; }


void PIDHandler::set_windup_clamp(Type min, Type max)
{
    if (_gains.ki == 0) 
    {
        return;
    }
    _is_clamp_set = true;
    _aw_clamp_min = min/_gains.ki;
    _aw_clamp_max = max/_gains.ki;
}

int PIDHandler::set_anti_windup(AntiWindupType type)
{
    if(type == AntiWindupType::CLAMP && !_is_clamp_set) { return 1; }

    _aw_type = type;
    return 0;
}

PIDHandler::Type PIDHandler::_clamp_float(Type value) 
{
    if(!_is_clamp_set) { return value;}
    else if (value > _aw_clamp_max) return _aw_clamp_max;
    else if (value < _aw_clamp_min) return _aw_clamp_min;
    return value;
}