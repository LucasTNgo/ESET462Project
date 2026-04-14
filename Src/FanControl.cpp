#include "FanControl.h"

FanControl* FanControl::_instances[30] = {nullptr};

FanControl::FanControl(uint pwm_pin, uint tach_pin, float pwm_freq, uint pulses_per_rev)
    : _pwm(pwm_pin, pwm_freq),
      _tach_pin(tach_pin),
      _ppr(pulses_per_rev)
{
}

void FanControl::init() {
    // ---- PWM setup ----
    _pwm.set_duty_cycle(0.0f);
    _pwm.enable_slice();
    _pwm.enable_pin();

    // ---- Tach setup ----
    gpio_init(_tach_pin);
    gpio_set_dir(_tach_pin, GPIO_IN);
    gpio_pull_up(_tach_pin);

    _instances[_tach_pin] = this;

    gpio_set_irq_enabled_with_callback(
        _tach_pin,
        GPIO_IRQ_EDGE_FALL,
        true,
        &FanControl::gpio_callback
    );
}

void FanControl::set_duty(float duty) {
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;

    _pwm.set_duty_cycle(duty);
    _pwm.enable_pin(); // update level
}

float FanControl::get_duty() {
    return _pwm.get_duty_cycle();
}

void FanControl::gpio_callback(uint gpio, uint32_t events) {
    if (_instances[gpio]) {
        _instances[gpio]->handle_interrupt();
    }
}

void FanControl::handle_interrupt() {
    uint32_t now = time_us_32();

    if ((now - _last_time) < MIN_PULSE_US) return;
    _last_time = now;

    _buf[_head] = now;
    _head = (_head + 1) % BUF_SIZE;

    if (_count < BUF_SIZE) _count++;
}

float FanControl::get_rpm() {
    if (_count < 2) return 0.0f;

    uint32_t local_buf[BUF_SIZE];
    uint8_t local_head, local_count;

    uint32_t irq = save_and_disable_interrupts();
    local_head = _head;
    local_count = _count;
    for (int i = 0; i < local_count; i++) {
        local_buf[i] = _buf[i];
    }
    restore_interrupts(irq);

    uint8_t newest_idx = (local_head - 1 + BUF_SIZE) % BUF_SIZE;
    uint8_t oldest_idx = (local_head - local_count + BUF_SIZE) % BUF_SIZE;

    uint32_t newest = local_buf[newest_idx];
    uint32_t oldest = local_buf[oldest_idx];

    uint32_t dt = newest - oldest;
    if (dt == 0) return 0.0f;

    float dt_sec = dt / 1e6f;

    float rpm = ((local_count - 1) / (float)_ppr) * (60.0f / dt_sec);

    return rpm;
}