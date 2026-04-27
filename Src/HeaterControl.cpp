#include "HeaterControl.h"
#include "hardware/gpio.h"

HeaterControl::HeaterControl(uint pin, float freq) : _pin(pin)
{
    _timer.user_data = this;
    add_repeating_timer_us(-(1000000 / (freq * TICKS)), _callback, this, &_timer);
}

void HeaterControl::init()
{
    gpio_init(_pin);
    gpio_set_dir(_pin, GPIO_OUT);
    gpio_put(_pin, 0);
}


float HeaterControl::get_duty() { return _duty; };
void HeaterControl::set_duty(float duty)
{
    _duty = duty;
    _ticks_on = (uint32_t)(duty * TICKS + 0.5f);
}

bool HeaterControl::_callback(repeating_timer_t *rt)
{
    auto *h = static_cast<HeaterControl*>(rt->user_data);
    h->_tick = (h->_tick + 1) % TICKS;
    gpio_put(h->_pin, h->_tick < h->_ticks_on ? 1 : 0);
    return true;
}