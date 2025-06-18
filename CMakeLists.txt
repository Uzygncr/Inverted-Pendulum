# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)


project(final C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME}
    main.c
    source/1_hardware_configuration.c
    source/2_blink.c
    source/2_encoder.c
    source/2_motor.c
    source/2_hall_sensor.c
    source/2_lcd.c
    source/2_buzzer.c
    source/3_menu.c
    source/3_calibration.c
    source/3_usb.c
    source/3_monitor.c
    source/3_talker.c
    source/4_swing_up.c
    source/4_lqr.c
    source/5_controller.c
    
)

# Define target Resources
target_include_directories(${PROJECT_NAME} PRIVATE include )


target_link_libraries(${PROJECT_NAME} 
    pico_cyw43_arch_lwip_threadsafe_background 
    pico_stdlib
    hardware_watchdog
    pico_multicore
    hardware_gpio   # GPIO library
    hardware_pwm    # PWM library
    hardware_uart   # Uart library
    hardware_timer
    hardware_i2c
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})
