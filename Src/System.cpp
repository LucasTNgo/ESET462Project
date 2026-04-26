#include "System.hpp"

float clamp_float(float value, float min, float max)
{
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

uint64_t get_time_us()
{
    return time_us_64();
}

// Call this periodically in your main loop (if USB used)
// uart = uart0 or uart1
// returns 1 when a new float is ready
int read_usb_float(float *out)
{
    static char buf[SERIAL_BUF_SIZE];
    static int idx = 0;

    while (true) {
        int c = getchar_timeout_us(0); // non-blocking

        if (c == PICO_ERROR_TIMEOUT)
            break;

        if (c == '\n' || c == '\r') {
            if (idx == 0) continue;

            buf[idx] = '\0';

            char *end;
            float val = strtof(buf, &end);

            if (end != buf) {
                *out = val;
                idx = 0;
                return 1;
            }

            idx = 0;
        }
        else {
            if (idx < 63) buf[idx++] = (char)c;
            else idx = 0;
        }
    }

    return 0;
}

float absolute_humidity(float temp_c, float rel_humidity)
{
    // Saturation vapor pressure (hPa)
    float es = 6.112f * expf((17.62f * temp_c) / (243.12f + temp_c));
    
    // Actual vapor pressure (hPa)
    float e = (rel_humidity / 100.0f) * es;
    
    // Absolute humidity (g/m^3)
    float ah = 216.7f * (e / (temp_c + 273.15f));
    
    return ah;
}

float relative_humidity(float temp_c, float abs_humidity)
{
    // Saturation vapor pressure (hPa)
    float es = 6.112f * expf((17.62f * temp_c) / (243.12f + temp_c));
    
    // Actual vapor pressure from absolute humidity (hPa)
    float e = (abs_humidity * (temp_c + 273.15f)) / 216.7f;
    
    // Relative humidity (%)
    float rh = (e / es) * 100.0f;

    return rh;
}
/*
float get_equiv_rhdiff(){
    return (int_reading.humidity-ext_equiv_rh)/100;
}
*/