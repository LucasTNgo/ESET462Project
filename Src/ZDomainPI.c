// ZDomainPI.c
#include "ZDomainPI.h"

static void recompute_coeffs(ZPI* c)
{
    c->b0 =  c->gains.kp + c->gains.ki * c->T / 2.0f;
    c->b1 = -c->gains.kp + c->gains.ki * c->T / 2.0f;
}

void zpi_init(ZPI* c, ZPI_Gains gains, float sample_period_s)
{
    c->T            = sample_period_s;
    c->gains        = gains;
    c->target       = 0.0f;
    c->prev_error   = 0.0f;
    c->integrator   = 0.0f;
    c->windup_min   = -1.0f;
    c->windup_max   =  1.0f;
    c->anti_windup  = ZPI_CLAMPTYPE_NONE;
    recompute_coeffs(c);
}

void zpi_reset(ZPI* c)
{
    c->prev_error = 0.0f;
    c->integrator = 0.0f;
}

void zpi_set_target(ZPI* c, float target)
{
    c->target = target;
}

void zpi_set_gains(ZPI* c, ZPI_Gains gains)
{
    c->gains = gains;
    recompute_coeffs(c);
}

void zpi_set_gains_interpolated(ZPI* c, ZPI_Gains a, ZPI_Gains b, float alpha)
{
    ZPI_Gains interp = {
        .kp = a.kp * (1.0f - alpha) + b.kp * alpha,
        .ki = a.ki * (1.0f - alpha) + b.ki * alpha
    };
    zpi_set_gains(c, interp);
}

void zpi_set_clamp(ZPI* c, float min, float max)
{
    c->windup_min = min;
    c->windup_max = max;
}

void zpi_set_clamp_type(ZPI* c, ZPI_ClampType type)
{
    c->anti_windup = type;
}

float zpi_tick(ZPI* c, float measurement)
{
    float error = c->target - measurement;

    c->integrator += c->b0 * error + c->b1 * c->prev_error;

    if (c->anti_windup == ZPI_CLAMPTYPE_CLAMP)
    {
        if (c->integrator > c->windup_max) c->integrator = c->windup_max;
        if (c->integrator < c->windup_min) c->integrator = c->windup_min;
    }

    c->prev_error = error;

    return c->integrator;
}