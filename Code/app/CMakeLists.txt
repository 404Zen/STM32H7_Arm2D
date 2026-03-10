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
set(CMAKE_PROJECT_NAME app)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Clang emits warnings for GCC-style __attribute__((optimize)) in CubeMX headers
# silence that globally rather than patching generated files
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-unknown-attributes)
endif()

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

add_library(arm2d_includes INTERFACE)

# Option to use prebuilt CMSIS-DSP library instead of building from source
set(CMSIS_DSP_PREBUILT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS-DSP_prebuilt" CACHE PATH "Path to prebuilt CMSIS-DSP library")

# Check if prebuilt library exists
if(EXISTS "${CMSIS_DSP_PREBUILT_PATH}/lib/libCMSISDSP.a" AND EXISTS "${CMSIS_DSP_PREBUILT_PATH}/include")
    # Use prebuilt CMSIS-DSP library
    message(STATUS "✓ Using prebuilt CMSIS-DSP from: ${CMSIS_DSP_PREBUILT_PATH}")
    set(CMSISDSP_INCLUDE_PATH "${CMSIS_DSP_PREBUILT_PATH}/include")
    
    add_library(CMSISDSP STATIC IMPORTED GLOBAL)
    set_target_properties(CMSISDSP PROPERTIES
        IMPORTED_LOCATION "${CMSIS_DSP_PREBUILT_PATH}/lib/libCMSISDSP.a"
        INTERFACE_INCLUDE_DIRECTORIES "${CMSISDSP_INCLUDE_PATH}"
        IMPORTED_NO_SONAME ON
    )
else()
    # Build CMSIS-DSP from the submodule with testing disabled
    message(STATUS "⚠ Prebuilt CMSIS-DSP not found, building from source...")
    set(CMSISDSP_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS-DSP/Include")
    set(CMSISDSP_BUILD_TESTING OFF CACHE BOOL "Disable CMSIS-DSP tests" FORCE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    add_subdirectory(CMSIS-DSP)
endif()

target_include_directories(arm2d_includes INTERFACE
    # ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/ARM/DSP/Inc
    ${CMSISDSP_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/Arm-2D_Port/config
)

set(ARM2D_HELPER ON)
set(ARM2D_LCD_PRINTF ON)
set(ARM2D_CONTROLS ON)

set(CMSISCORE "${CMAKE_SOURCE_DIR}/Drivers/CMSIS" CACHE STRING "Path to CMSIS Core")
# set(CMSISCORE "${CMAKE_SOURCE_DIR}/Drivers/CMSIS" CACHE STRING "Path to CMSIS Core" FORCE)
add_compile_definitions(__PERF_COUNTER__)  

add_compile_definitions(
    VECT_TAB_SRAM
    USER_VECT_TAB_ADDRESS
)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)
add_subdirectory(cmake/perf_counter)
add_subdirectory(cmake/arm2d)

# Special optization for Arm-2D library
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(ARM2D PRIVATE -O3 -ffast-math -g0)
else()
    target_compile_options(ARM2D PRIVATE -O3)
endif()

target_link_libraries(ARM2D PUBLIC arm2d_includes)


# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    App/Common/ring_buffer.c
    App/Drivers/async_uart.c
    App/Drivers/key.c
    App/Drivers/time_port.c
    App/Drivers/io_i2c.c
    App/Drivers/GT9xx_touch.c
    Arm-2D_Port/adapter/arm_2d_disp_adapter_0.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    App/Common
    App/Drivers
    perf_counter/
    Arm-2D_Port/config
    Arm-2D_Port/adapter
    Arm-2D/Library/Include
    Arm-2D/Helper/Include
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    RTE_Acceleration_Arm_2D_Helper_Disp_Adapter0
    RTE_Acceleration_Arm_2D_Extra_LCD_printf
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    # Add user defined libraries
    ARM2D
    CMSISDSP       # DSP runtime for math functions used by ARM2D helper
    perf_counter
    stm32cubemx
)
