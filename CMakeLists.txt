cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME gpio_work)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        Mission/device/MPU6050/MPU6050.cpp
        Mission/device/MPU6050/MPU6050.h
        Mission/control_task/mission_planner.cpp
        Mission/control_task/task/init_task.cpp
        Mission/control_task/task/mpu6050_task.cpp
        Mission/algorithm/EKF/ekf.cpp
        Mission/algorithm/EKF/ekf.h
        Mission/Core/rw_lock.cpp
        Mission/Core/rw_lock.h
        Mission/device/SPL/spl_reg.h
        Mission/device/SPL/SPL.cpp
        Mission/device/SPL/SPL.h
        Mission/control_task/task/spl06_task.cpp
        Mission/control_task/task/pwm_task.cpp
        Mission/Peripheral/UART/uart_drv.cpp
        Mission/Peripheral/UART/uart_drv.h
        Mission/task/attitude_task.h
        Mission/task/attitude_task.cpp
)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
        Mission/Core/FSM.c
        Mission/device/MPU6050/MPU6050.cpp
        Mission/algorithm/EKF/ekf.cpp
        Mission/Core/rw_lock.cpp
        Mission/device/SPL/SPL.cpp
        Mission/Peripheral/UART/uart_drv.cpp
        Mission/task/attitude_task.cpp
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
        Mission/device/MPU6050
        Mission/device/SPL
        Mission/algorithm/EKF
        Mission/Core
        Mission/Peripheral/UART
        Mission/task
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
