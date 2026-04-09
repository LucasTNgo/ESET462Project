#include "TempHandlers.h"

void* setup_sht4x(i2c_inst_t *i2c, char port_code){
    void *ctx = NULL;
    uint8_t i2c_addr;
    switch(port_code)
    {
        case 'A':
        case 'a':
            i2c_addr = 0x44;
            break;
        case 'B':
        case 'b':
            i2c_addr = 0x45;
            break;
        case 'C':
        case 'c':
            i2c_addr = 0x46;
            break;
        default:
            printf("Invalid port code");
            return NULL;
    }

    int res = i2c_init_sensor(get_i2c_sensor_type("SHT4x"), i2c, i2c_addr, &ctx);
    switch(res) //Check success state
    {
        case 0:
            break; //Success
        case 1:
            printf("Invalid Parameters");
            break;
        case 2:
            printf("Device not found");
            break;
        case 3:
            printf("Device failed to initialize (Check device type)");
            break;
        default:
            printf("Unexpected State");
    }
    return ctx;
}

SensorReading get_sensor_reading(void* ctx)
{
    SensorReading readings = {0.0f, 0.0f, 0.0f};
    int delay = i2c_start_measurement(ctx);
    if (delay < 0) {
        printf("Failed to initiate measurement");
        return readings;
    }
    sleep_ms(delay);


    int res = i2c_read_measurement(ctx, &readings.temp, &readings.pressure, &readings.humidity);
    if (res) {
        printf("Measurement error");
    }
    return readings;
}

float ntc_temperature_c(uint8_t adc_port)
{
    
    adc_select_input(adc_port);
    uint16_t raw_ntc1 = adc_read();

    // Constants (tweak if needed)
    const float VREF = 3.3f;
    const float ADC_MAX = 4095.0f;

    const float R_FIXED = 4700.0f;    // 4k7
    const float R0 = 100000.0f;       // 100k @ 25C
    const float B = 3950.0f;
    const float T0 = 298.15f;         // 25C in Kelvin

    // Convert ADC to voltage
    float v = (raw_ntc1 / ADC_MAX) * VREF;

    // Avoid divide-by-zero edge cases
    if (v <= 0.0f) return -273.15f;
    if (v >= VREF) return -273.15f;

    // Solve divider for NTC resistance
    float r_ntc = (v * R_FIXED) / (VREF - v);

    // Beta equation
    float temp_k = 1.0f / ( (1.0f / T0) + (1.0f / B) * logf(r_ntc / R0) );

    return temp_k - 273.15f; // Celsius
}