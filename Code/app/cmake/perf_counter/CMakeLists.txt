cmake_minimum_required(VERSION 3.22)
project(perf_counter)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMSISCORE "" CACHE STRING "Path to CMSIS Core")


set(PERF_COUNTER_SOURCES
    ../../perf_counter/perf_counter.c
    ../../perf_counter/perfc_port_default.c
    ../../perf_counter/perfc_port_pmu.c
    ../../perf_counter/perfc_task_coroutine.c
)


add_library(perf_counter STATIC ${PERF_COUNTER_SOURCES})

target_include_directories(perf_counter PUBLIC ${CMSISCORE}/Include)


target_include_directories(perf_counter 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ../../perf_counter/
)

# Optional
set_target_properties(perf_counter PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS ON
)