#include "Application.hpp"
#include "ApplicationConfig.h"
#include "RTOSTasks.hpp"
#include "ZDomainPI.h"

namespace Application
{
    static ZPI heater_controller;
    static ZPI fan_controller;

    static ZPI_Gains heater_gains;
    static ZPI_Gains heater_with_fan_gains;
    static ZPI_Gains fan_gains;

    static bool is_init = false;

    int init()
    {
        if (is_init) return 0;
        is_init = true;

        // Controller Gains
        heater_gains          = { .kp = HEATER_KP,     .ki = HEATER_KI     };
        heater_with_fan_gains = { .kp = HEATER_FAN_KP, .ki = HEATER_FAN_KI };
        fan_gains             = { .kp = FAN_KP,        .ki = FAN_KI        };

        // Heater
        zpi_init(&heater_controller, heater_gains, APP_SAMPLE_PERIOD_S);
        zpi_set_target(&heater_controller, HEATER_TARGET);
        zpi_set_clamp(&heater_controller, HEATER_CLAMP_MIN, HEATER_CLAMP_MAX);

        // Fan
        zpi_init(&fan_controller, fan_gains, APP_SAMPLE_PERIOD_S);
        zpi_set_target(&fan_controller, FAN_TARGET);
        zpi_set_clamp(&fan_controller, FAN_CLAMP_MIN, FAN_CLAMP_MAX);

        return 0;
    }

    Hardware::SystemActuatorDuties update(const Hardware::SystemReadings& state)
    {
        if (state.int_reading.error != 0 || state.ext_reading.error != 0)
            return {};

        float fan_effort  = zpi_tick(&fan_controller, get_rh_diff(state));
        float fan_duty    = clamp_float(-fan_effort, 0.0f, 1.0f);

        // Dead zone - fan doesn't behave below minimum
        if (fan_duty < FAN_DUTY_MIN)
            fan_duty = 0.0f;

        zpi_set_gains_interpolated(
            &heater_controller,
            heater_gains,
            heater_with_fan_gains,
            fan_duty
        );

        float heater_effort = zpi_tick(&heater_controller, get_temperature(state));
        float heater_duty   = clamp_float(heater_effort, HEATER_CLAMP_MIN, HEATER_CLAMP_MAX);

        return {
            .heater_duty = heater_duty,
            .fan_duty    = fan_duty
        };
    }

    float get_temperature(const Hardware::SystemReadings& state)
    {
        return state.int_reading.temp;
    }

    float get_rh_diff(const Hardware::SystemReadings& state)
    {
        return (state.int_reading.humidity - state.ext_equiv_rh) / 100.0f;
    }
}