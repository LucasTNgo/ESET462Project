#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "FanControl.h"

static constexpr int   FAN_PWM              = 13;
static constexpr int   FAN_TACH             = 26;

static constexpr float SETTLE_TOLERANCE_RPM = 20.0f;
static constexpr int   SETTLE_SAMPLES       = 5;
static constexpr int   SAMPLE_DELAY_MS      = 100;
static constexpr int   SWEEP_STEPS          = 20;
static constexpr float STARTUP_SCAN_STEP    = 0.01f;
static constexpr float STARTUP_SCAN_MAX     = 0.60f;
static constexpr float STARTUP_RPM_THRESH   = 200.0f;


static float wait_for_stable(FanControl& fan)
{
    float last  = 0.0f;
    float sum   = 0.0f;
    int   count = 0;

    while (count < SETTLE_SAMPLES) {
        float rpm = fan.get_rpm();

        if (fabs(rpm - last) < SETTLE_TOLERANCE_RPM) {
            sum += rpm;
            count++;
        } else {
            sum   = 0.0f;
            count = 0;
        }

        last = rpm;
        sleep_ms(SAMPLE_DELAY_MS);
    }

    return sum / SETTLE_SAMPLES;
}

static void sweep(FanControl& fan, int from, int to)
{
    int step = (to > from) ? 1 : -1;

    for (int i = from; i != to + step; i += step) {
        float duty = i / (float)SWEEP_STEPS;
        fan.set_duty(duty);
        float rpm = wait_for_stable(fan);
        printf("%.3f,%.1f\n", duty, rpm);
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    FanControl fan(FAN_PWM, FAN_TACH);
    fan.init();

    printf("duty,rpm\n");

    // Scan for startup threshold
    fan.set_duty(0.0f);
    sleep_ms(2000);

    for (float duty = STARTUP_SCAN_STEP; duty <= STARTUP_SCAN_MAX; duty += STARTUP_SCAN_STEP) {
        fan.set_duty(duty);
        sleep_ms(500);

        if (fan.get_rpm() > STARTUP_RPM_THRESH) {
            printf("# START_DUTY=%.3f\n", duty);
            break;
        }
    }

    // Sweep up then down for hysteresis
    sweep(fan, 0,           SWEEP_STEPS);
    sweep(fan, SWEEP_STEPS, 0);

    printf("# DONE\n");

    for (;;) sleep_ms(1000);
}