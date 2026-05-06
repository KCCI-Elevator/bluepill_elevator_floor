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
set(CMAKE_PROJECT_NAME blue_pill_can_comm)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        MyApp/Common/def.h
        MyApp/Driver/can_comm.c
        MyApp/Driver/oled_spi.c
        MyApp/Driver/oled_spi.h
        MyApp/Common/font.h
        MyApp/Driver/button.c
        MyApp/Driver/button.h
        MyApp/BSP/bsp.c
        MyApp/BSP/bsp.h
        MyApp/Common/hw_def.h
        MyApp/Application/app.c
        MyApp/Application/app.h
        MyApp/Driver/servo.c
        MyApp/Driver/servo.h
        MyApp/Driver/my_adc.c
        MyApp/Driver/my_adc.h
        MyApp/Driver/ts0224.c
        MyApp/Driver/ts0224.h)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        MyApp/Common
        MyApp/Driver
        MyApp/BSP
        MyApp/Hardware
        MyApp/Application
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
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
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
