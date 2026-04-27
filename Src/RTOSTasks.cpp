#include "RTOSTasks.hpp"
#include <stdio.h>

extern "C" {
    #include "pico/time.h"
    #include "ApplicationConfig.h"
}

#define SENSOR_PERIOD_MS    100
#define CONTROL_PERIOD_MS   APP_SAMPLE_PERIOD_MS       // Defined In ApplicationConfig.h 
#define ACTUATOR_PERIOD_MS  1667
#define DEBUG_PERIOD_MS     2000

namespace
{
    constexpr TickType_t SENSOR_DELAY   = pdMS_TO_TICKS(SENSOR_PERIOD_MS);
    constexpr TickType_t CONTROL_DELAY  = pdMS_TO_TICKS(CONTROL_PERIOD_MS);
    constexpr TickType_t ACTUATOR_DELAY = pdMS_TO_TICKS(ACTUATOR_PERIOD_MS);
    constexpr TickType_t DEBUG_DELAY    = pdMS_TO_TICKS(DEBUG_PERIOD_MS);
}

namespace RTOSTasks
{
    QueueHandle_t readings_queue = nullptr;
    QueueHandle_t duties_queue   = nullptr;

    int init()
    {
        readings_queue = xQueueCreate(1, sizeof(Hardware::SystemReadings));
        duties_queue   = xQueueCreate(1, sizeof(Hardware::SystemActuatorDuties));

        if (!readings_queue || !duties_queue)
        {
            printf("Queue creation failed\n");
            return -1;
        }

        BaseType_t ok = pdPASS;
        ok &= xTaskCreate(sensor_task,   "sensor",   512,  nullptr, 3, nullptr);
        ok &= xTaskCreate(control_task,  "control",  1024, nullptr, 2, nullptr);
        ok &= xTaskCreate(actuator_task, "actuator", 512,  nullptr, 2, nullptr);
        ok &= xTaskCreate(debug_task,    "debug",    512,  nullptr, 1, nullptr);

        if (ok != pdPASS)
        {
            printf("Task creation failed\n");
            return -1;
        }

        return 0;
    }

    // -------------------------
    // SENSOR TASK
    // Highest priority — reads hardware and publishes snapshot
    // -------------------------
    void sensor_task(void*)
    {
        Hardware::SystemReadings readings;

        while (true)
        {
            readings = Hardware::read_state();
            xQueueOverwrite(readings_queue, &readings);
            vTaskDelay(SENSOR_DELAY);
        }
    }

    // -------------------------
    // CONTROL TASK
    // Waits for fresh readings, runs PID, publishes duties
    // -------------------------
    void control_task(void*)
    {
        Hardware::SystemReadings readings;
        Hardware::SystemActuatorDuties duties;

        while (true)
        {
            // Block until sensor_task posts a reading
            if (xQueuePeek(readings_queue, &readings, pdMS_TO_TICKS(200)) == pdTRUE)
            {
                duties = Application::update(readings);
                xQueueOverwrite(duties_queue, &duties);
            }
            else
            {
                printf("control_task: no sensor data\n");
            }

            vTaskDelay(CONTROL_DELAY);
        }
    }

    // -------------------------
    // ACTUATOR TASK
    // Applies latest duties to hardware at its own rate
    // -------------------------
    void actuator_task(void*)
    {
        Hardware::SystemActuatorDuties duties;

        while (true)
        {
            if (xQueuePeek(duties_queue, &duties, pdMS_TO_TICKS(200)) == pdTRUE)
            {
                Hardware::set_actuators(duties);
            }

            vTaskDelay(ACTUATOR_DELAY);
        }
    }

    // -------------------------
    // DEBUG TASK
    // Read-only observer, prints CSV at low rate
    // -------------------------
    void debug_task(void*)
    {
        Hardware::SystemReadings readings;
        Hardware::SystemActuatorDuties duties;

        UserDebug::csv_print_header();

        while (true)
        {
            vTaskDelay(DEBUG_DELAY);

            if (xQueuePeek(readings_queue, &readings, 0) == pdTRUE &&
                xQueuePeek(duties_queue,   &duties,   0) == pdTRUE)
            {
                uint32_t time_ms = to_ms_since_boot(get_absolute_time());

                // Use this for csv
                //UserDebug::csv_print(time_ms, readings, duties);

                // Use this for verbose
                UserDebug::debug_print(time_ms, readings, duties, 0.f);
            }
        }
    }
}