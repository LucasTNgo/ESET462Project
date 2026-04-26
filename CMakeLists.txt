cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico2_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)

project(SHT40Test C CXX ASM)

pico_sdk_init()

# FreeRTOS - must be before add_executable
# FreeRTOS - must be before add_executable
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE Inc)
target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)
target_link_libraries(freertos_config INTERFACE pico_base_headers)

include(FreeRTOS_Kernel_import.cmake)

add_subdirectory(Libs/pico-sensor-lib)
add_subdirectory(Tools/FanCharacterization)
add_subdirectory(Tools/HeaterCharacterization)

add_executable(SHT40Test 
    Src/main.cpp 
    Src/TempHandlers.cpp
    Src/PWMHandlers.cpp
    Src/FanControl.cpp
    Src/HeaterControl.cpp
    Src/PIDHandler.cpp
    Src/System.cpp
)

pico_set_program_name(SHT40Test "SHT40Test")
pico_set_program_version(SHT40Test "0.1")

pico_enable_stdio_uart(SHT40Test 0)
pico_enable_stdio_usb(SHT40Test 1)

target_include_directories(SHT40Test PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
)

target_link_libraries(SHT40Test
    pico_stdlib
    hardware_i2c
    hardware_adc
    hardware_pwm
    pico_cyw43_arch_none
    pico_sensor_lib
    freertos_config
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)

pico_add_extra_outputs(SHT40Test)