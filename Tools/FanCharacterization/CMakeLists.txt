add_executable(FanCharacterization 
    FanCharacterization.cpp 
    ../../Src/PWMHandlers.cpp
    ../../Src/FanControl.cpp
)

target_link_libraries(FanCharacterization
    pico_stdlib
    hardware_pwm
)

target_include_directories(FanCharacterization PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Inc
)

pico_add_extra_outputs(FanCharacterization)