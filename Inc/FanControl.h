#pragma once

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/sync.h"
#include "PWMHandlers.h"

class FanControl {
public:
    FanControl(uint pwm_pin, uint tach_pin, float pwm_freq = 25000.0f, uint pulses_per_rev = 2);

    void init();

    void set_duty(float duty);   // 0.0 - 1.0
    float get_duty();

    float get_rpm();
    uint32_t _tach_count;

private:
    // PWM
    PWMPin _pwm;

    // Tach
    uint _tach_pin;
    uint _ppr;

    static constexpr uint BUF_SIZE = 8;

    volatile uint32_t _buf[BUF_SIZE];
    volatile uint8_t _head = 0;
    volatile uint8_t _count = 0;
    volatile uint32_t _last_time = 0;

    static constexpr uint32_t MIN_PULSE_US = 200;

    // ISR plumbing
    static FanControl* _instances[30];
    static void gpio_callback(uint gpio, uint32_t events);
    void handle_interrupt();
};