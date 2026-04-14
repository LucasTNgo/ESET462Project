#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "FanControl.h"
#include "hardware/clocks.h"


static constexpr int   FAN_PWM              = 13;
static constexpr int   FAN_TACH             = 26;


int main()
{
    stdio_init_all();
    sleep_ms(5000);

    FanControl fan(FAN_PWM, FAN_TACH);
    fan.init();

    uint32_t freq = clock_get_hz(clk_sys);
    printf("sys clock: %lu Hz\n", freq);

    printf("duty, rpm\n");

    constexpr float STEP_SIZE = 1.0f/100.0f;
    constexpr int NUM_SAMPLES = 5;

    float rpm = 0.0f;
    for(float sweepDuty = 0; sweepDuty < 1.0f; sweepDuty += STEP_SIZE)
    {
        fan.set_duty(sweepDuty);
        sleep_ms(2000);
        for(int i = 0; i < NUM_SAMPLES; i++)
        {
            rpm = fan.get_rpm();
            printf("%.2f, %.4f, %d\n", sweepDuty, rpm, fan._tach_count);
            sleep_ms(500);
        }
    }

    for (;;) sleep_ms(1000);
}