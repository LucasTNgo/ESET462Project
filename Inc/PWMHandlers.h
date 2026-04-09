#pragma once
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

class PWMPin
{
    private:
        uint pin;
        uint slice;
        uint channel;
        float dutyCycle;
        bool pinEnabled;

        static float sliceFreqs[8];
        static bool sliceEnabled[8];

    public:
        PWMPin(uint pin, float freq);

        uint get_pin();
        float get_freq();
        uint get_slice();
        float get_clkdiv();
        float get_duty_cycle();
        bool is_slice_enabled();
        bool is_pin_enabled();
        
        void set_freq(float freq);
        void set_duty_cycle(float duty);
        void enable_slice();
        void disable_slice();
        void enable_pin();
        void disable_pin();
};