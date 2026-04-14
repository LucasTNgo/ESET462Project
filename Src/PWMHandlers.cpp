#include "PWMHandlers.h"

#define SYS_CLK 150000000.0f

float PWMPin::sliceFreqs[8] = {0};
bool PWMPin::sliceEnabled[8] = {0};

uint    PWMPin::get_pin()           { return this->pin; }
float   PWMPin::get_freq()          { return PWMPin::sliceFreqs[this->slice]; }
uint    PWMPin::get_slice()         { return this->slice; }
float   PWMPin::get_clkdiv()        { return SYS_CLK / (this->get_freq() * 65536.0f); }
float   PWMPin::get_duty_cycle()    { return this->dutyCycle; }
bool    PWMPin::is_slice_enabled()        { return this->sliceEnabled[this->slice]; }
bool    PWMPin::is_pin_enabled()        { return this->pinEnabled; }

PWMPin::PWMPin(uint pin, float freq)
{
    this->pin = pin;
    gpio_set_function(this->pin, GPIO_FUNC_PWM);
    
    this->slice = pwm_gpio_to_slice_num(pin);
    this->channel = pwm_gpio_to_channel(pin);

    // Set frequency
    this->set_freq(freq);
}

void PWMPin::set_freq(float freq)
{
// Set frequency
    this->sliceFreqs[this->slice] = freq;
    float divider = SYS_CLK / (freq * 65536.0f);
    pwm_set_clkdiv(this->slice, divider);
    pwm_set_wrap(this->slice, 65535);
}

void PWMPin::set_duty_cycle(float duty) { this->dutyCycle = duty; }

void PWMPin::enable_slice()     
{
    this->sliceEnabled[this->slice] = true; 
    pwm_set_enabled(this->slice, true);
}
void PWMPin::disable_slice()
{
    this->sliceEnabled[this->slice] = false;
    pwm_set_enabled(this->slice, false);
}

void PWMPin::enable_pin()
{
    this->pinEnabled = true;
    pwm_set_chan_level(this->slice, this->channel, (uint16_t)(this->dutyCycle * 65535));
}
void PWMPin::disable_pin()
{
    this->pinEnabled = false;
    pwm_set_chan_level(this->slice, this->channel, 0);
}