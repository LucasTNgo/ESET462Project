#include "UserDebug.hpp"
#include <stdio.h>

namespace UserDebug
{
    void debug_print(
        uint32_t  time_ms,
        const Hardware::SystemReadings& state,
        const Hardware::SystemActuatorDuties& duties,
        float fan_rpm
    )
    {
        printf("\nTime (ms): %d =========\n", time_ms);

        printf("Internal:\tTemp: %0.2f\tRH: %0.2f%%\n",
               state.int_reading.temp,
               state.int_reading.humidity);

        printf("External:\tTemp: %0.2f\tRH: %0.2f%%\teRH: %0.2f\t deltaRH: %0.2f\n",
               state.ext_reading.temp,
               state.ext_reading.humidity,
               state.ext_equiv_rh,
               state.rh_diff);

        printf("Fan duty: %0.2f, Fan RPM: %.2f\n",
               duties.fan_duty,
               fan_rpm);

        printf("Heater duty: %0.3f\n",
               duties.heater_duty);
    }

    void csv_print_header()
    {
        printf("time_ms,temp_internal,humidity_internal,rh_diff,heater_duty,fan_duty\n");
    }

    void csv_print(
        uint32_t  time_ms,
        const Hardware::SystemReadings& state,
        const Hardware::SystemActuatorDuties& duties
    )
    {
        printf("%d,%.3f,%.3f,%.3f,%.3f,%.3f\n",
               time_ms,
               state.int_reading.temp,
               state.int_reading.humidity,
               state.rh_diff,
               duties.heater_duty,
               duties.fan_duty);
    }
}