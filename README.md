# STM32H7_Arm2D


### Memory Map

```Plaintext
	 Address          Memory Structure (Size)                  		Detail Partition               Address
  [0xFFFFFFFF] +---------------------------------------+---------------------------------------+
               |                                       |									   |
  0x9080_0000  +---------------------------------------+---------------------------------------+ 0x9080_0000
               |                                       |         Read_Only_Data (6144K)        |
               |         External_FLASH (8192K)        +---------------------------------------+ 0x9020_0000
               |                                       |           Application (2048K)         |
  0x9000_0000  +---------------------------------------+---------------------------------------+ 0x9000_0000
               |            Reserved/Unused            |            Reserved/Unused            |
  0x2410_0000  +---------------------------------------+---------------------------------------+
               |             AXI_SRAM3 (384K)          |									   |
  0x240A_0000  +---------------------------------------+---------------------------------------+
               |             AXI_SRAM2 (384K)          |									   |
  0x2404_0000  +---------------------------------------+---------------------------------------+
               |             AXI_SRAM1 (256K)          |									   |
  0x2400_0000  +---------------------------------------+---------------------------------------+
               |                Reserved               |                                       |
  0x2002_0000  +---------------------------------------+---------------------------------------+
               |             DTCM_RAM (128K)           |                                       |
  0x2000_0000  +---------------------------------------+---------------------------------------+
               |                Reserved               |									   |
  0x1FF2_0000  +---------------------------------------+---------------------------------------+
               |            System_RAM (128K)          |									   |
  0x1FF0_0000  +---------------------------------------+---------------------------------------+
               |                Reserved               |									   |
  0x0802_0000  +---------------------------------------+---------------------------------------+ 0x0802_0000
               |                                       |          Read_Write_Data (16K)        |
               |                                       +---------------------------------------+ 0x0801_C000
               |         Internal_FLASH (128K)         |          App_Fast_Code (80K)          |
               |                                       +---------------------------------------+ 0x0800_8000
               |                                       |               Boot (32K)              |
  0x0800_0000  +---------------------------------------+---------------------------------------+ 0x0800_0000
               |                Reserved               |									   |
  0x0001_0000  +---------------------------------------+---------------------------------------+
               |              ITCM_RAM (64K)           |									   |
  0x0000_0000  +---------------------------------------+---------------------------------------+
```









### COPY VECTOR TABLE TO SRAM

```ld
  /* The startup code goes first into FLASH */
    /* used by the startup to initialize vector table */
  _sivtor = LOADADDR(.isr_vector);
  .isr_vector :
  {
    . = ALIGN(4);
    _svtor = .;
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
    _evtor = .; 
  } >RAM AT> EXT_FLASH
```





```assembly
/* Copy the vector segment from flash to SRAM */
  ldr r0, =_svtor
  ldr r1, =_evtor
  ldr r2, =_sivtor
  movs r3, #0 
  b LoopCopyVtorInit

CopyVtorInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4
    
LoopCopyVtorInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyVtorInit
```





# DMA2D

`480*800*2 = 768000 `~750KB

RAM split two part, last 768KB use as GRAM.(AXI_SRAM2 & AXI_SRAM3)

RAM (xrw)           : ORIGIN = 0x24000000, LENGTH = 256K
GRAM (xrw)          : ORIGIN = 0x24040000, LENGTH = 768K





# ARM-2D

### Add submodules and update all submodules.

```shell
git submodule add https://github.com/ARM-software/Arm-2D.git Code/app/Arm-2D
git submodule add https://github.com/GorgonMeducer/perf_counter.git Code/app/perf_counter
git submodule update --init --recursive
```

### Add cmake for ARM-2D and perf_counter

Create CmakeLists.txt in cmake/arm2d and cmake/perf_counter directories for Arm-2D and perf_counter.

In the top CmakeLists.txt, add above libraries into your project.

```cmake
add_library(arm2d_includes INTERFACE)
target_include_directories(arm2d_includes INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/ARM/DSP/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Arm-2D_Port/config
)

set(ARM2D_HELPER ON)
set(ARM2D_LCD_PRINTF ON)
set(ARM2D_CONTROLS ON)

set(CMSISCORE "${CMAKE_SOURCE_DIR}/Drivers/CMSIS" CACHE STRING "Path to CMSIS Core")
# set(CMSISCORE "${CMAKE_SOURCE_DIR}/Drivers/CMSIS" CACHE STRING "Path to CMSIS Core" FORCE)
add_compile_definitions(__PERF_COUNTER__)  


add_subdirectory(cmake/perf_counter)
add_subdirectory(cmake/arm2d)

# Special optization for Arm-2D library
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(ARM2D PRIVATE -O3 -ffast-math -g0)
else()
    target_compile_options(ARM2D PRIVATE -O3)
endif()

target_link_libraries(ARM2D PUBLIC arm2d_includes)


# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    # Add user defined libraries
    ARM2D
    perf_counter
)
```



### Add DSP Library

Arm-2D depend on DSP Library, so we need add DSP in this project.

~~Install STM32 X-CUBE-ALGOBUILD pack and then selected DSP Library in STM32CubeMX~~(Detail in Problems #3)

**DO NOT** use a folder named **`Middlewares`** for your own CMSIS-DSP library. If you use libraries from STM32CubeMX, the tool may overwrite or delete your custom files in that directory. 

### Add CMSIS-DSP

`git submodule add https://github.com/ARM-software/CMSIS-DSP.git`



```CMAKE
add_library(arm2d_includes INTERFACE)
target_include_directories(arm2d_includes INTERFACE
    # ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/ARM/DSP/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS-DSP/Include  #include arm_math.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Arm-2D_Port/config
)

add_subdirectory(CMSIS-DSP)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    # Add user defined libraries
    ARM2D
    CMSISDSP       # DSP runtime for math functions used by ARM2D helper
    perf_counter
    stm32cubemx
)
```



### ARM2D Init

add dwt related code

```C
bool check_dwt_enabled(void)  
{  
    // 检查 Debug Exception and Monitor Control Register  
    uint32_t demcr = CoreDebug->DEMCR;  
    bool debug_enabled = (demcr & CoreDebug_DEMCR_TRCENA_Msk) != 0;  
      
    // 检查 DWT Control Register  
    uint32_t dwt_ctrl = DWT->CTRL;  
    bool dwt_enabled = (dwt_ctrl & DWT_CTRL_CYCCNTENA_Msk) != 0;  
      
    // printf("DEMCR: 0x%08lX (TRCENA=%s)\r\n",   
    //        demcr, debug_enabled ? "ON" : "OFF");  
    // printf("DWT_CTRL: 0x%08lX (CYCCNTENA=%s)\r\n",   
    //        dwt_ctrl, dwt_enabled ? "ON" : "OFF");  
      
    return debug_enabled && dwt_enabled;  
}

void enable_dwt(void)  
{  
    // 启用 Debug Exception and Monitor Control Register  
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  
      
    // 启用 DWT cycle counter  
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;  
      
    // 重置计数器  
    DWT->CYCCNT = 0;  
      
    // printf("DWT enabled\r\n");  
}
```



init perf_counmter and ARM2D

```C
if(check_dwt_enabled() ==  false)
  {
    enable_dwt();
  }
  perfc_init(true);
  arm_irq_safe 
  {
    arm_2d_init();
  }

  /* initialize the display adapter 0 service */
  disp_adapter0_init();
```



Call  disp_adapter0_task() in main loop.

```C
tResult = disp_adapter0_task();
```



and do not forget add perf_counter's systick handler in your SysTick_Handler()

```
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  user_code_insert_to_systick_handler();
  /* USER CODE END SysTick_IRQn 1 */
}
```



On STM32H7B0, we use *LCD Direct Mode*, so __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__ in arm_2d_disp_adapter_0.h need set to 1

```C
// <q>Enable the helper service for 3FB (LCD Direct Mode)
// <i> You can select this option when your LCD controller supports direct mode
#ifndef __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__
#   define __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__                 1
#endif
```

> 后面我发现，3FB 其实并不适用于我的配置



`IMPL_PFB_ON_DRAW(__pfb_draw_handler)` define the content show in display

```C
static
IMPL_PFB_ON_DRAW(__pfb_draw_handler)
{
    ARM_2D_PARAM(pTarget);
    ARM_2D_PARAM(ptTile);

    arm_2d_canvas(ptTile, __top_container) {
    
#if __DISP0_CFG_COLOR_SOLUTION__ != 1              /* as long as it is not monochrome */
        arm_2d_align_centre(__top_container, 100, 100) {
            draw_round_corner_box(  ptTile,
                                    &__centre_region,
                                    GLCD_COLOR_BLACK,
                                    64);
        }
#endif

        busy_wheel2_show(ptTile, bIsNewFrame);
    }

    arm_2d_op_wait_async(NULL);

    return arm_fsm_rt_cpl;
}
```

After that, compile and download execute file to MCU， and then we can see a busy wheel show in LCD.





# LTDC & DMA2D

基于受限的单屏 GRAM (即只有 Layer0)的条件下理解的 LTDC 与 DMA2D。



## LTDC

在我们配置了 LTDC 之后，LTDC 会周而复始的将 GRAM* 中的内容搬运到 LCD 中；所以我们只需要修改 GRAM 中的数据，LCD 上显示的内容便会产生变化。

此时，屏幕的刷新率由以下公式决定。

`刷新率 = 像素时钟频率 / [(水平总周期) × (垂直总周期)]`

根据我的屏幕数据手册，像素时钟典型值是 30MHz，最大值是 50MHz。

我们先测试一下当前配置下，ARM2D 的测试数据。

| LTDC Clock | FPS  | Rendering time | LCD Latency |
| ---------- | ---- | -------------- | ----------- |
| 30MHz      | 54   | 18ms           | 14ms        |
| 50MHz      | 54   | 18ms           | 14ms        |

可以看到的是，平均帧时间约为 18ms，LCD 延迟为 14 ms，所以渲染一帧花费的时间是 4ms。

> 这里我有点疑问的是，为什么 LTDC 时钟加到了 50MHz， **LCD Latency 没有下，难道和目前没有正确使用 ARM2D 有关系？**
>
> 根据下面的计算，如果时钟速率是 30MHz，可以达到 61.39FPS，消隐时间是 1.425ms；得出来的单帧传输时间约为 14.86ms
>
> 如果时钟速率是 50MHz，则可以达到 102.32FPS，消隐时间是 0.855ms ；**得出来的单帧传输时间约为 8.92ms**



根据当前的屏幕配置

```
水平总周期 = TotalWidth + 1 = 88(HBP) + 800(Active Width) + 40(HFP) + 1 = 928 + 1 = 929
垂直总周期 = TotalHeight + 1 = 32(VBP) + 480(Active Height) + 13(VFP) = 525 + 1 = 526;
刷新率 = 像素时钟频率 / [(水平总周期) × (垂直总周期)]
```

 所以在实现 60Hz 刷新率的情况下，我们的 LTDC 时钟需要 29.32 MHz， 之前的 30MHz 已经足以满足需求。

VSYNC+VBP+VFP 决定了 垂直消隐时间，在这个时间里，LTDC 不会刷屏，对于 GRAM 中数据的更改，这个时间内是安全的。

消隐时间计算：

```
VBLANKING = VSYNC+VBP+VFP
HORIZONTAL_TOTAL = 929
VBLANKING*HORIZONTAL_TOTAL = (1 + 32 + 13) * 929 

VBLANKING_TIME = VBLANKING*HORIZONTAL_TOTAL/LTDC_CLK

when LTDC_CLK = 30MHz

VBLANKING_TIME = 46*929/30000000 = 1.4245 mS
```

所以，可以通过适当修改 VBP/VFP 来调整消隐时间，当然这也会对刷新率造成影响。



### 怎么开启 STM32H7B0 的消隐中断

对于 STM32H7B0,  LTDC_IT_RR 中断的作用与 VSYNC 中断的作用一致。

```c
  __HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_LI | LTDC_FLAG_FU | LTDC_FLAG_TE | LTDC_FLAG_RR);
  HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
  __HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_RR);
```

需要注意的是， `HAL_LTDC_IRQHandler()` 会关闭这个中断；所以我们在渲染数据更新完成之后，需要手动再次开启这个中断。

**⚠️ 注意 ： 此处数据更新和中断开启应该放到主循环中而不是在 中断回调里， 此处是为了文档撰写的逻辑通顺暂时放在这里。**

```
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc)
{
  if(hltdc->Instance == LTDC)
  {
    // if((hltdc->ISR & LTDC_ISR_RRIF) != 0U)
    if(hltdc->State == HAL_LTDC_STATE_READY)
    {
      /* Call function in main.c to update frame buffer for next transfer */
      // UpdateLayeredBuffer(); 
      // after frame buffer updated, you need enable the reload interrupt to trigger next reload event
      __HAL_LTDC_CLEAR_FLAG(hltdc, LTDC_FLAG_LI | LTDC_FLAG_FU | LTDC_FLAG_TE | LTDC_FLAG_RR);
      HAL_LTDC_Reload(hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
      __HAL_LTDC_ENABLE_IT(hltdc, LTDC_IT_RR);
    }
  }
}
```



***GRAM : 指的是在 SRAM 中划分出的一块专们给 LCD 所使用的空间。**

## DMA2D





























# Problems & Bugs

1. `.ARM.exidx` limit range in ± 1G， limit place code in internal flash(0x0800_8000) and external flash(0x9000_0000).

   ```shell
   [build] ld.lld: error: <internal>:(.ARM.exidx+0x8): relocation R_ARM_PREL31 out of range: 1811987350 is not in [-1073741824, 1073741823]
   ```

   **Workaround: **discard in link file, maybe not a good idea(**I'm not familiar with this**). then we can place codes in internal flash when it need fast speed.

   ```ld
   /DISCARD/ :
   {
     libc.a:* ( * )
     libm.a:* ( * )
     libgcc.a:* ( * )
     *(.ARM.exidx*)   /* Remove ARM exception unwinding index table */
     *(.ARM.extab*)   /* Remove ARM exception unwinding instructions */
   }
   ```
   
1. When you select the DSP library in STM32CubeMX’s code generation tool, it does **not** copy the pre-compiled library file (`arm_cortexM7l_math.a`) or the DSP library source files to the user project.

   I'm not sure if this is an intended **feature** or a **bug**. The same problem has **appeared** on both Windows and macOS (tested with STM32CubeMX versions 6.16.0 and 6.17.0).
   
   **Solution:** Therefore, the workaround is to **build the CMSIS-DSP library yourself**. You can find the source files on GitHub, or copy them from the STM32CubeMX library installation path. 

   > https://github.com/ARM-software/CMSIS-DSP

3. Sometimes, external Flash files fail to download, which will cause a hardfault; the reason is currently unclear. Based on this, on launch.json, i moved the app before the boot, which might help.

   **Workaround : ** Nothing can do now.

4. In the VSCode STM32Cube Build Analyzer (v1.1.0) extension, there appear to be display errors regarding the **ITCM section** (address 0x0000_0000). The extension includes sections such as `.ARM.attributes` and `.symtab` in its usage calculation **which should be excluded**, leading to a reported usage higher than the actual amount. However, the ITCM section usage is **correctly reflected** in the `.map` file.

​	<img src="assets/image-20260201233005767.png" alt="image-20260201233005767" style="zoom: 67%;" /><img src="assets/image-20260201233026332.png" alt="image-20260201233026332" style="zoom:67%;" />

​	**Workaround:** wait for updates.....

5. LCD screen abnormalities

   **Solution:**  The output speed of the pins used by the LTDC must be configured to **Very High**; otherwise, screen corruption may occur after a period of operation.

6. 下载完成之后，可能会进入 MemFault()；怀疑是 SFL 没有清理现场导致的，我尝试修改了一些 SFL 的相关代码，但是没有效果。

   **Workaround:** 在下载完成之后，执行一次复位，launch.json 中添加 preRunCommands 暂时可以绕过这个问题。

   ```json
   "preRunCommands": [
                   // reset clr MemMangement Fault
                   "printf \"Reset MCU after download\\n\"",
                   "monitor reset",
                   "monitor sleep 100"
                   ],
   ```

   

7. 一定概率 boot 的 会卡在 OSPI 初始化阶段

   **Solution:** boot 在初始化 OSPI 之前，添加 反初始化 代码确保 OSPI 被正确复位。

8. 












# TODOS

- [x] External flash loader

- [x] boot

- [ ] application
  - [x] Uart async ring buffer
    - [x] Transmit use software ring buffer (It doesn't seem to work very well when sending large amounts of data... 
      will fix it future if necessary )
    
    - [x] Receive use DMA circular mode
    
  - [x] Button
  
  - [ ] ~~LEDs control~~
  
  - [ ] ARM2D
  
  - [ ] 
  



