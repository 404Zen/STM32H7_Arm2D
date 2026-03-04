cmake_minimum_required(VERSION 3.22)
project(ARM2D)

option(ARM2D_HOST "Build for host" OFF)
option(ARM2D_HELPER "Build with helper support" OFF)
option(ARM2D_CMSIS_RTOS2 "Build with CMSIS-RTOS2 helper support" OFF)
option(ARM2D_RT_THREAD "Build with RT-Thread helper support" OFF)
option(ARM2D_LCD_PRINTF "Build with LCD printf support" OFF)
option(ARM2D_CONTROLS "Build with controls" OFF)
option(ARM2D_BENCHMARK_GENERIC "Build with generic benchmark support" OFF)
option(ARM2D_BENCHMARK_WATCHPANEL "Build with watchpanel benchmark support" OFF)
set(CMSISCORE "" CACHE STRING "Path to CMSIS Core")

add_library(ARM2D STATIC)

if (ARM2D_HOST)
    target_compile_definitions(ARM2D PUBLIC HOST)

    target_compile_options(ARM2D PUBLIC 
    -DARM_SECTION\(x\)= 
    -D__va_list=va_list
    -fms-extensions)

   target_compile_options(ARM2D PRIVATE -Wno-macro-redefined)
else() 
    target_include_directories(ARM2D PUBLIC ${CMSISCORE}/Include)
    # target_compile_options(ARM2D PRIVATE -Wno-nonnull-compare -Wno-unused-value)
    target_compile_options(ARM2D PRIVATE -Wno-null-pointer-subtraction -Wno-unused-value)
    target_compile_options(ARM2D PUBLIC -fms-extensions)

endif()


target_compile_options(ARM2D PUBLIC
                       -DRTE_Acceleration_Arm_2D
                       -DRTE_Acceleration_Arm_2D_Alpha_Blending
                       -DRTE_Acceleration_Arm_2D_Transform
                       -DRTE_Acceleration_Arm_2D_Filter
                       )
target_compile_options(ARM2D PRIVATE -Wno-format
        -Wno-unused-function)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ARM2D PRIVATE -Wno-maybe-uninitialized)
endif()
    
target_include_directories(ARM2D PRIVATE 
                            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Library/Include/)

target_sources(ARM2D PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Library/Source/arm_2d.c
	${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Library/Source/arm_2d_async.c
	${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Library/Source/arm_2d_draw.c
	${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Library/Source/arm_2d_conversion.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Library/Source/arm_2d_helium.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Library/Source/arm_2d_alpha_blending.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Library/Source/arm_2d_transform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Library/Source/arm_2d_filter.c
    )


if (ARM2D_HELPER)
	target_include_directories(ARM2D PUBLIC 
                                ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Include/
                                ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/built_in/
                                ${CMAKE_CURRENT_SOURCE_DIR}/../../perf_counter/
                                ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D_Port/adapter/
                                )

	target_sources(ARM2D PRIVATE 
		    ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_helper.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_helper_pfb.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_helper_scene.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_helper_list.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_helper_shape.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_helper_control.c

            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_asset_white_dot_small.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_asset_cmsis_logo.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_asset_circle_small.c

            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_helper_built_in.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/busy_wheel.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/console_box.c
            
            )
    target_compile_options(ARM2D PUBLIC
            -DRTE_Acceleration_Arm_2D_Helper_PFB
            )
endif()

if (ARM2D_CMSIS_RTOS2)
	target_sources(ARM2D PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/template/arm_2d_helper_rtos_cmsis_rtos2.c)
    target_compile_options(ARM2D PUBLIC
        -DRTE_Acceleration_Arm_2D_Helper_RTOS_CMSIS_RTOS2
        )
endif()

if (ARM2D_RT_THREAD)
	target_sources(ARM2D PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/template/arm_2d_helper_rtos_rt_thread.c)
    target_compile_options(ARM2D PUBLIC
        -DRTE_Acceleration_Arm_2D_Helper_RTOS_RT_Thread
        )
endif()

if (ARM2D_LCD_PRINTF)
	target_include_directories(ARM2D PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/)

	target_sources(ARM2D PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_helper_font.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/Helper/Source/arm_2d_asset_glcd_fonts.c
            )
    target_compile_options(ARM2D PUBLIC
            -DRTE_Acceleration_Arm_2D_Extra_LCD_printf__
            )
endif()

if (ARM2D_TJPGD_LOADER)
	target_include_directories(ARM2D PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/loader/tjpgd_loader)

	target_sources(ARM2D PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/opcodes/arm_2d_user_opcode_draw_circle.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/opcodes/arm_2d_user_opcode_draw_line.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/loader/tjpgd_loader/tjpgd_loader.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/loader/tjpgd_loader/tjpgd/tjpgd.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/loader/zjpgd_loader/zjpgd_loader.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/loader/zjpgd_loader/zjpgd/zjpgd.c
            )
    target_compile_options(ARM2D PUBLIC
            -DRTE_Acceleration_Arm_2D_Extra_TJpgDec_Loader
            -DRTE_Acceleration_Arm_2D_Extra_ZJpgDec_Loader
            -DRTE_Acceleration_Arm_2D_Extra_JPEG_Loader
            -DRTE_Acceleration_Arm_2D_Extra_Loader
            )
endif()

if (ARM2D_CONTROLS)
	target_include_directories(ARM2D PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/)
	target_sources(ARM2D PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/controls.c
            
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/spinning_wheel.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/progress_bar_drill.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/progress_bar_flowing.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/progress_bar_simple.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/progress_bar_round.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/__simple_list.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/number_list.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/text_list.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/text_tracking_list.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/progress_wheel.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/list_view.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/battery_gasgauge.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/dynamic_nebula.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/histogram.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/spin_zoom_widget.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/cloudy_glass.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/meter_pointer.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/text_box.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/qrcode_box/qrcode_box.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/qrcode_box/qrcodegen.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/crt_screen.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/image_box.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/controls/foldable_panel.c

            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_WhiteDotMiddle.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_WhiteDotBig.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_SpinWheel.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_blue_slashes.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_wave.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_GreenCircle.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_QuaterArcSmall.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_QuaterArcMiddle.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_QuaterArcBig.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_ListCover.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_battery_boarder_1.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_battery_gasgauge_block.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_battery_gasgauge_grade_boarder.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_SinWave.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_glass_reflection_wide.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_glass_reflection_narrow.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_Lighting.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_Pointer.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_watch_panel.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_pointer_sec.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_gear_01.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_gear_02.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_star.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_small_icon_sun.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_software.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_circle_mask.c
            
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_wifi_signal.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_helium.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_background.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_BackgroundSmall.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_WatchCoverRound.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_GlassHalo.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_alarm_clock_font32.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_alarm_clock_font64.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_ECG.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_ECGScan.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_fan_blade.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_RadialGradient.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_GlassBall.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_HallowOutCircleMask.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_Fire8x4.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_DigitsFont.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_alarm_clock_font32.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_alarm_clock_font64.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_liberalsans_regular_ascii_font14.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_liberalsans_regular_ascii_font32.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_liberalsans_regular_ascii_font64.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_ClockMarking.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_compass.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_Earth.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_knob_circle_quater.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_radial_line_cover.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_radial_line_pattern.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_wedge_pattern.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_GlassBall40.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_doge.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_doge_small.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_white_noise.c

            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_automobile.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_compute.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_Developer.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_drone.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_Durability.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_enterprise.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_healthcare.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_mobile.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_multicore.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_smartmeter.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_visual_sensor.c 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/asset/arm_2d_asset_icon_wearables.c

          )
    target_compile_options(ARM2D PUBLIC
          -DRTE_Acceleration_Arm_2D_Extra_Controls
          )
endif()

if (ARM2D_BENCHMARK_GENERIC)
	target_include_directories(ARM2D PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/benchmark/)
	target_sources(ARM2D PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/benchmark/benchmark_generic.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/benchmark/arm_2d_scene_benchmark_generic.c
			${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/benchmark/arm_2d_scene_benchmark_generic_cover.c
          )
    target_compile_options(ARM2D PUBLIC
          -DRTE_Acceleration_Arm_2D_Extra_Benchmark
          -DRTE_Acceleration_Arm_2D_Extra_Benchmark_Generic
          )
endif()

if (ARM2D_BENCHMARK_WATCHPANEL)
	target_include_directories(ARM2D PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/benchmark/)
	target_sources(ARM2D PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/benchmark/benchmark_watch_panel.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/benchmark/arm_2d_scene_benchmark_watch_panel.c
			${CMAKE_CURRENT_SOURCE_DIR}/../../Arm-2D/examples/common/benchmark/arm_2d_scene_benchmark_watch_panel_cover.c
          )
    target_compile_options(ARM2D PUBLIC
          -DRTE_Acceleration_Arm_2D_Extra_Benchmark
          -DRTE_Acceleration_Arm_2D_Extra_Benchmark_Watch_Panel
          )
endif()

target_include_directories(ARM2D PUBLIC Library/Include/)