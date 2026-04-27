extern "C" {
    #include "pico.h"
    #include "pico/stdlib.h"
    #include "pico/cyw43_arch.h"
    #include "FreeRTOS.h"
    #include "task.h"
}

#include <stdio.h>
#include "Hardware.hpp"
#include "Application.hpp"
#include "RTOSTasks.hpp"

void startupTask(void *params)
{
    if (cyw43_arch_init()) {
        printf("WiFi init failed\n");
        while (true);
    }

    if (Hardware::init() != 0) {
        printf("Hardware init failed\n");
        while (true);
    }

    if (Application::init() != 0) {
        printf("Application init failed\n");
        while (true);
    }

    if (RTOSTasks::init() != 0) {
        printf("RTOS init failed\n");
        while (true);
    }

    printf("Startup complete\n");
    vTaskDelete(NULL);
}

int main()
{
    stdio_init_all();
    sleep_ms(4000);
    printf("Booting\n");

    xTaskCreate(startupTask, "startup", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();

    while (true);
}