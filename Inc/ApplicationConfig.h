#pragma once

// ApplicationConfig.hpp
#pragma once

// -------------------------
// Sample rate (must match CONTROL_PERIOD_MS in RTOSTasks.hpp)
// -------------------------
#define APP_SAMPLE_PERIOD_MS    100
#define APP_SAMPLE_PERIOD_S     (APP_SAMPLE_PERIOD_MS / 1000.0f)

// -------------------------
// Heater PI gains
// -------------------------
#define HEATER_KP               0.0806963431668464f
#define HEATER_KI               6.2770230388285e-05f

#define HEATER_FAN_KP           0.0848136068469526f
#define HEATER_FAN_KI           6.59728735270854e-05f

#define HEATER_CLAMP_MIN        0.0f
#define HEATER_CLAMP_MAX        1.0f

#define HEATER_TARGET           50.0f

// -------------------------
// Fan PI gains
// -------------------------
#define FAN_KP                  2.0585f
#define FAN_KI                  0.078125f

#define FAN_CLAMP_MIN           -1.0f
#define FAN_CLAMP_MAX           0.0f

#define FAN_TARGET              0.03f
#define FAN_DUTY_MIN            0.3f    // fan doesn't behave below this