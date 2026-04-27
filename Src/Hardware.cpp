#include "Hardware.hpp"


// TODO: Set error returns
// Returns 0
namespace Hardware {
    
    static void* SHT40_1 = nullptr;
    static void* SHT40_2 = nullptr;

    static FanControl* fan = nullptr;
    static HeaterControl* heater = nullptr;
    static bool is_init = false;

    int init() 
    {
        if(is_init)
        {
            return 0; // already initialized, but no problem
        }
        is_init = true;

        
        fan = new FanControl(FAN_PWM, FAN_TACH);
        fan->init();
        fan->set_duty(0.0f);

        heater = new HeaterControl(HEATER_PIN, HEATER_FREQ);
        heater->init();
        heater->set_duty(0.25f);

        //Init I2C
        i2c_setup(I2C0_PORT, I2C0_SDA, I2C0_SCL, 400 * 1000);
        i2c_setup(I2C1_PORT, I2C1_SDA, I2C1_SCL, 400 * 1000);

        /*
        //Init ADC pins
        adc_init();
        adc_gpio_init(GPIO_NTC1); // ADC0
        adc_gpio_init(GPIO_NTC2); // ADC1
        */

        // Example to turn on the Pico W LED
        //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

        SHT40_1 = setup_sht4x(i2c0, 'A'); // setup_sht4x generates a pointer handle for the sensor
        SHT40_2 = setup_sht4x(i2c1, 'A');

        return 0;
    }


    



    // TODO: Set error returns
    SystemReadings read_state()
    {
        SystemReadings state;

        /*
        //Get NTC readings
        float temp_NTC1 = ntc_temperature_c(ADC_NTC1);
        */ 

        //Get SHT40 Readings
        state.int_reading = get_sensor_reading(SHT40_1);
        state.ext_reading = get_sensor_reading(SHT40_2);
        if (state.int_reading.error == 0 && state.ext_reading.error == 0)
        {
            // Calculate external equivalent humidity
            state.ext_equiv_rh = relative_humidity(state.int_reading.temp, absolute_humidity(state.ext_reading.temp, state.ext_reading.humidity));
        }
        state.rh_diff = state.int_reading.humidity - state.ext_equiv_rh;

        return state;
    }

    void set_actuators(SystemActuatorDuties efforts)
    {
        heater->set_duty(efforts.heater_duty);
        fan->set_duty(efforts.fan_duty);
    }

}

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


void i2c_setup(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint baudrate)
{
    // Initialize I2C peripheral at given baudrate
    i2c_init(i2c, baudrate);

    // Set GPIO function to I2C
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    // Enable internal pull-ups (important for I2C bus idle state)
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}