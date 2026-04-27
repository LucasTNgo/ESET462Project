#pragma once

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
}

#include "Hardware.hpp"
#include "Application.hpp"
#include "UserDebug.hpp"

namespace RTOSTasks
{
    // -------------------------
    // Mailboxes (single-slot queues)
    // -------------------------
    extern QueueHandle_t readings_queue;
    extern QueueHandle_t duties_queue;

    // -------------------------
    // Task entry points
    // -------------------------
    void sensor_task(void* params);
    void control_task(void* params);
    void actuator_task(void* params);
    void debug_task(void* params);

    // -------------------------
    // RTOS setup
    // -------------------------
    int init();
}