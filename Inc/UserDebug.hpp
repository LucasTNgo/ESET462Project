
#pragma once

#include "Hardware.hpp"

namespace UserDebug
{
    void debug_print(
        uint32_t  time_ms,
        const Hardware::SystemReadings& state,
        const Hardware::SystemActuatorDuties& duties,
        float fan_rpm
    );

    void csv_print_header();

    void csv_print(
        uint32_t  time_ms,
        const Hardware::SystemReadings& state,
        const Hardware::SystemActuatorDuties& duties
    );
}