#pragma once

extern "C" {
    #include "pico.h"
    #include "pico/stdlib.h"
}

#include "PIDHandler.h"
#include "Hardware.hpp"

namespace Application
{
    // Lifecycle
    int init();

    // Main control update (pure input → output)
    Hardware::SystemActuatorDuties update(const Hardware::SystemReadings& state);
    int update_efforts();

    // Sensor-derived helper functions (no hidden globals)
    float get_temperature(const Hardware::SystemReadings& state);
    float get_rh_diff(const Hardware::SystemReadings& state);
}