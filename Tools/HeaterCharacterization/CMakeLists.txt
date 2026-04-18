add_executable(HeaterCharacterization 
    HeaterCharacterization.cpp 
    ../../Src/PWMHandlers.cpp
    ../../Src/TempHandlers.cpp
    ../../Src/HeaterControl.cpp
)

target_link_libraries(HeaterCharacterization
    pico_stdlib
    pico_sensor_lib
    hardware_pwm
    hardware_clocks
    hardware_i2c
    hardware_adc
)

target_include_directories(HeaterCharacterization PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Inc
)

pico_enable_stdio_uart(HeaterCharacterization 0)
pico_enable_stdio_usb(HeaterCharacterization 1)

pico_add_extra_outputs(HeaterCharacterization)