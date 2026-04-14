add_executable(FanCharacterization 
    FanCharacterization.cpp 
    ../../Src/PWMHandlers.cpp
    ../../Src/FanControl.cpp
)

target_link_libraries(FanCharacterization
    pico_stdlib
    hardware_pwm
    hardware_clocks
)

target_include_directories(FanCharacterization PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Inc
)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(FanCharacterization 0)
pico_enable_stdio_usb(FanCharacterization 1)

pico_add_extra_outputs(FanCharacterization)