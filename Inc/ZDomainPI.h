// ZDomainPI.h
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float kp;
    float ki;
} ZPI_Gains;

typedef enum {
    ZPI_CLAMPTYPE_NONE,
    ZPI_CLAMPTYPE_CLAMP
} ZPI_ClampType;

typedef struct {
    // Coefficients
    float b0;
    float b1;

    // Config
    float T;            // sample period in seconds
    float target;
    float windup_min;
    float windup_max;
    ZPI_ClampType anti_windup;

    // State
    float prev_error;
    float integrator;

    // Gains (stored for interpolation)
    ZPI_Gains gains;
} ZPI;

// Lifecycle
void zpi_init(ZPI* c, ZPI_Gains gains, float sample_period_s);
void zpi_reset(ZPI* c);

// Config
void zpi_set_target(ZPI* c, float target);
void zpi_set_gains(ZPI* c, ZPI_Gains gains);
void zpi_set_gains_interpolated(ZPI* c, ZPI_Gains a, ZPI_Gains b, float alpha);
void zpi_set_clamp(ZPI* c, float min, float max);
void zpi_set_clamp_type(ZPI* c, ZPI_ClampType type);

// Run
float zpi_tick(ZPI* c, float measurement);

#ifdef __cplusplus
}
#endif