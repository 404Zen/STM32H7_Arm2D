# STM32H7_Arm2D

[English](README.md) | [中文](README.zh-CN.md)

## Table of Contents / 目录

- [内存映射](#内存映射)
- [将向量表复制到 SRAM](#将向量表复制到-sram)
- [DMA2D](#dma2d)
- [ARM-2D](#arm-2d)
- [LTDC & DMA2D](#ltdc--dma2d)
- [问题与 Bug](#问题与-bug)
- [TODO](#todo)

## 内存映射

```Plaintext
	 Address          Memory Structure (Size)                  		Detail Partition               Address
  [0xFFFFFFFF] +---------------------------------------+---------------------------------------+
               |                                       |							   |
  0x9080_0000  +---------------------------------------+---------------------------------------+ 0x9080_0000
               |                                       |         Read_Only_Data (6144K)        |
               |         External_FLASH (8192K)        +---------------------------------------+ 0x9020_0000
               |                                       |           Application (2048K)         |
  0x9000_0000  +---------------------------------------+---------------------------------------+ 0x9000_0000
               |            Reserved/Unused            |            Reserved/Unused            |
  0x2410_0000  +---------------------------------------+---------------------------------------+
               |             AXI_SRAM3 (384K)          |                                       |
  0x240A_0000  +---------------------------------------+               GRAM (750K)              |
               |        AXI_SRAM2 upper (366K)         |                                       |
  0x2404_4800  +---------------------------------------+---------------------------------------+ 0x2404_4800
               |        AXI_SRAM2 lower (18K)          |                                       |
  0x2404_0000  +---------------------------------------+           RAM_NOCACHE (274K)           | 0x2404_0000
               |             AXI_SRAM1 (256K)          |                                       |
  0x2400_0000  +---------------------------------------+---------------------------------------+ 0x2400_0000
               |                Reserved               |                                       |
  0x2002_0000  +---------------------------------------+---------------------------------------+
               |             DTCM_RAM (128K)           |                                       |
  0x2000_0000  +---------------------------------------+---------------------------------------+
               |                Reserved               |							   |
  0x1FF2_0000  +---------------------------------------+---------------------------------------+
               |            System_RAM (128K)          |							   |
  0x1FF0_0000  +---------------------------------------+---------------------------------------+
               |                Reserved               |							   |
  0x0802_0000  +---------------------------------------+---------------------------------------+ 0x0802_0000
               |                                       |          Read_Write_Data (16K)        |
               |                                       +---------------------------------------+ 0x0801_C000
               |         Internal_FLASH (128K)         |          App_Fast_Code (80K)          |
               |                                       +---------------------------------------+ 0x0800_8000
               |                                       |               Boot (32K)              |
  0x0800_0000  +---------------------------------------+---------------------------------------+ 0x0800_0000
               |                Reserved               |							   |
  0x0001_0000  +---------------------------------------+---------------------------------------+
               |              ITCM_RAM (64K)           |							   |
  0x0000_0000  +---------------------------------------+---------------------------------------+
```

## 将向量表复制到 SRAM

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
} >RAM_NOCACHE AT> EXT_FLASH
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

`480*800*2 = 768000 bytes`（约 750 KB）

SRAM 划分为两部分：

- `RAM_NOCACHE`：用于非缓存数据和部分运行时段。
- `GRAM`：作为 LCD 帧缓冲区（位于 SRAM 末端 750KB）。

```Plaintext
RAM_NOCACHE (xrw)   : ORIGIN = 0x24000000, LENGTH = 274K
GRAM (xrw)          : ORIGIN = 0x24044800, LENGTH = 750K
```

# ARM-2D

### 添加子模块并更新所有子模块

```shell
git submodule add https://github.com/ARM-software/Arm-2D.git Code/app/Arm-2D
git submodule add https://github.com/GorgonMeducer/perf_counter.git Code/app/perf_counter
git submodule update --init --recursive
```

### 为 ARM-2D 和 perf_counter 添加 CMake

在 `cmake/arm2d` 和 `cmake/perf_counter` 目录中分别创建 `CMakeLists.txt`。

在顶层 `CMakeLists.txt` 中将这些库加入工程：

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

# Special optimization for Arm-2D library
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

### 添加 DSP 库

Arm-2D 依赖 DSP 库，因此项目中必须引入 DSP 支持。

~~安装 STM32 X-CUBE-ALGOBUILD，并在 STM32CubeMX 中勾选 DSP Library~~（详见“问题与 Bug”第 2 条）。

**不要**把你自己的 CMSIS-DSP 放在名为 **`Middlewares`** 的目录下。如果使用 STM32CubeMX 生成代码，该目录中的自定义文件可能会被覆盖或删除。

### 添加 CMSIS-DSP

```shell
git submodule add https://github.com/ARM-software/CMSIS-DSP.git
```

```cmake
add_library(arm2d_includes INTERFACE)
target_include_directories(arm2d_includes INTERFACE
    # ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/ARM/DSP/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS-DSP/Include  # include arm_math.h
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

### ARM2D 初始化

添加 DWT 相关代码：

```c
bool check_dwt_enabled(void)
{
    // 检查 DEMCR（Debug Exception and Monitor Control Register）
    uint32_t demcr = CoreDebug->DEMCR;
    bool debug_enabled = (demcr & CoreDebug_DEMCR_TRCENA_Msk) != 0;

    // 检查 DWT 控制寄存器
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
    // 使能 DEMCR
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // 使能 DWT 周期计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // 清零计数器
    DWT->CYCCNT = 0;

    // printf("DWT enabled\r\n");
}
```

初始化 `perf_counter` 与 ARM2D：

```c
if (check_dwt_enabled() == false)
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

在主循环中调用 `disp_adapter0_task()`：

```c
tResult = disp_adapter0_task();
```

不要忘记在 `SysTick_Handler()` 中加入 perf_counter 的 systick 处理：

```c
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

在 STM32H7B0 上我们使用 *LCD Direct Mode*，因此在 `arm_2d_disp_adapter_0.h` 中将 `__DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__` 设为 `1`：

```c
// <q>Enable the helper service for 3FB (LCD Direct Mode)
// <i> You can select this option when your LCD controller supports direct mode
#ifndef __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__
#   define __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__                 1
#endif
```

> 后续验证发现：3FB 实际并不适合当前最终配置。

`IMPL_PFB_ON_DRAW(__pfb_draw_handler)` 决定了实际显示内容：

```c
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

完成后编译并下载到 MCU，即可在 LCD 上看到 busy wheel。

# LTDC & DMA2D

本节在单屏 GRAM（只有 Layer0）的约束下说明 LTDC 与 DMA2D 的关系。

## LTDC

LTDC 配置完成后，会持续将 GRAM 内容搬运到 LCD，因此只要修改 GRAM 数据，显示内容就会改变。

刷新率计算公式：

`刷新率 = 像素时钟频率 / [(水平总周期) × (垂直总周期)]`

根据屏幕手册，像素时钟典型值为 30MHz，最大值为 50MHz。

当前配置下 ARM2D 测试结果：

| LTDC Clock | FPS | Rendering time | LCD Latency |
| ---------- | --- | -------------- | ----------- |
| 30MHz      | 54  | 18ms           | 14ms        |
| 50MHz      | 54  | 18ms           | 14ms        |

平均帧时间约 18ms，LCD 延迟约 14ms，因此渲染耗时约 4ms。

> 一个未完全解释的问题：LTDC 时钟升到 50MHz 后，LCD Latency 并没有下降，可能与当前 ARM2D 使用方式有关。
>
> 按下面计算：
>
> - 30MHz：理论 61.39 FPS，消隐时间 1.425ms，单帧传输约 14.86ms。
> - 50MHz：理论 102.32 FPS，消隐时间 0.855ms，单帧传输约 8.92ms。

当前屏幕参数：

```Plaintext
水平总周期 = TotalWidth + 1 = 88(HBP) + 800(Active Width) + 40(HFP) + 1 = 928 + 1 = 929
垂直总周期 = TotalHeight + 1 = 32(VBP) + 480(Active Height) + 13(VFP) + 1 = 525 + 1 = 526
刷新率 = 像素时钟频率 / [(水平总周期) × (垂直总周期)]
```

因此实现 60Hz 时，LTDC 时钟只需 29.32MHz，之前的 30MHz 已足够。

`VSYNC + VBP + VFP` 决定垂直消隐时间。该时间段 LTDC 不主动刷新，修改 GRAM 相对安全。

消隐时间计算：

```Plaintext
VBLANKING = VSYNC + VBP + VFP
HORIZONTAL_TOTAL = 929
VBLANKING * HORIZONTAL_TOTAL = (1 + 32 + 13) * 929

VBLANKING_TIME = VBLANKING * HORIZONTAL_TOTAL / LTDC_CLK

when LTDC_CLK = 30MHz

VBLANKING_TIME = 46 * 929 / 30000000 = 1.4245 mS
```

所以可以通过调节 VBP/VFP 改变消隐时间，但会同时影响刷新率。

### 如何开启 STM32H7B0 的消隐中断

对于 STM32H7B0，`LTDC_IT_RR` 中断与前面提到的 VSYNC 中断作用一致。

```c
__HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_LI | LTDC_FLAG_FU | LTDC_FLAG_TE | LTDC_FLAG_RR);
HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
__HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_RR);
```

需要注意：`HAL_LTDC_IRQHandler()` 会关闭这个中断，所以每次完成数据更新后，都要手动再次开启。

**⚠️ 注意：数据更新和中断重开应该放在主循环，而不是中断回调。这里放在回调示例只是为了文档逻辑连贯。**

```c
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc)
{
  if (hltdc->Instance == LTDC)
  {
    if (hltdc->State == HAL_LTDC_STATE_READY)
    {
      // 在 main.c 中更新下一帧数据
      // UpdateLayeredBuffer();

      // 更新完成后，重新打开 reload 中断以触发下一次事件
      __HAL_LTDC_CLEAR_FLAG(hltdc, LTDC_FLAG_LI | LTDC_FLAG_FU | LTDC_FLAG_TE | LTDC_FLAG_RR);
      HAL_LTDC_Reload(hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
      __HAL_LTDC_ENABLE_IT(hltdc, LTDC_IT_RR);
    }
  }
}
```

\*GRAM：在 SRAM 中划出的一块专用于 LCD 帧缓冲的内存区域。

## DMA2D

DMA2D 主要使用三个函数：

```c
HAL_StatusTypeDef HAL_DMA2D_Start(DMA2D_HandleTypeDef *hdma2d, uint32_t pdata, uint32_t DstAddress, uint32_t Width,
                                  uint32_t Height);
HAL_StatusTypeDef HAL_DMA2D_BlendingStart(DMA2D_HandleTypeDef *hdma2d, uint32_t SrcAddress1, uint32_t SrcAddress2,
                                          uint32_t DstAddress, uint32_t Width, uint32_t Height);
HAL_StatusTypeDef HAL_DMA2D_CLUTStartLoad(DMA2D_HandleTypeDef *hdma2d, const DMA2D_CLUTCfgTypeDef *CLUTCfg,
                                          uint32_t LayerIdx);
```

DMA2D 操作的地址都在 SRAM。完整流程通常是：

1. DMA2D 修改 GRAM。
2. LTDC 把 GRAM 刷到 LCD。
3. LTDC 一屏传输完成后触发 reload event（消隐阶段）。
4. 在消隐期间再让 DMA2D 写下一帧数据。

DMA2D 并非必须只在消隐期写数据，但在有效显示期写入容易出现撕裂，实战中建议避免。

这三个函数的最终目标都是修改 GRAM：

- `HAL_DMA2D_Start`
- `HAL_DMA2D_BlendingStart`
- `HAL_DMA2D_CLUTStartLoad`

`HAL_DMA2D_Start` 常见两种模式：

- 寄存器模式：`pdata` 表示颜色值，`DstAddress` 是目标地址。
- 内存到内存模式：`pdata` 表示源地址。

用红色填充矩形（寄存器模式）：

```c
HAL_DMA2D_Start(&hdma2d,
                0xFF0000,                    // 颜色（寄存器模式）
                (uint32_t)dest_address,      // 目标地址
                100, 50);                    // 宽, 高
```

复制一张图片（内存到内存模式）：

```c
HAL_DMA2D_Start(&hdma2d,
                (uint32_t)src_address,       // 源地址
                (uint32_t)dest_address,      // 目标地址
                200, 150);                   // 宽, 高
```

`HAL_DMA2D_BlendingStart` 用于带 Alpha 的图像混合：

```Plaintext
目标像素 = (前景像素 × 前景 Alpha)
         + (背景像素 × (1 - 前景 Alpha))
```

每层 Alpha 参数通过 `DMA2D_LayerCfgTypeDef` 配置。

`HAL_DMA2D_CLUTStartLoad` 用于把颜色查找表（CLUT）从内存加载到 DMA2D 的 CLUT 存储器。

CLUT 的核心用途是将较小位宽的颜色索引（如 4bit/8bit）映射到真实 RGB 颜色，从而减少内存占用和带宽。

示例：

```c
// 定义 16 色调色板
uint32_t clut_table[16] = {
    0x000000,  // 黑色
    0xFF0000,  // 红色
    0x00FF00,  // 绿色
    0x0000FF,  // 蓝色
    // ... 其他颜色
};

// 配置图层使用 CLUT
hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_A4;  // 4bit 索引模式
hdma2d.LayerCfg[0].CLUTColorMode = DMA2D_CCM_8888;   // CLUT 格式 ARGB8888
hdma2d.LayerCfg[0].CLUTSize = 15;                    // CLUT 大小 - 1

// 加载 CLUT 到 layer 0
HAL_DMA2D_CLUTStartLoad(&hdma2d,
    (uint32_t*)clut_table,  // CLUT 表地址
    0,                      // 图层索引
    16);                    // 颜色数量

// 等待完成
HAL_DMA2D_PollForTransfer(&hdma2d, 100);
```

若在 800×480 屏幕上显示 100×100 的 256 色图标：

- RGB565 + `HAL_DMA2D_Start`：`100*100*2 = 20,000` 字节。
- CLUT 方案：
  1. 先加载 CLUT（`256*4 = 1024` 字节）。
  2. 再传索引图（`100*100*1 = 10,000` 字节）。

因此在重复传输场景中，CLUT 方案带宽明显更小。

## 显示加速

在理解 LTDC 与 DMA2D 后，需要结合 ARM2D 修改代码以提升性能。

### 修正当前实现

当前 ARM2D 启用了 3FB，不符合实际需求。先按 ARM2D 文档切换到单缓冲正确流程。

设置为：

- `__ARM_2D_HAS_ASYNC__ = 0`
- `__DISP0_CFG_PFB_BLOCK_WIDTH__ = 800`
- `__DISP0_CFG_PFB_BLOCK_HEIGHT__ = 48`（1/10 屏高的缓存）
- `__DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__ = 0`

并修改 `Disp0_DrawBitmap()`，确保每次 `OutputOffset` 与 `DstAddress` 计算正确：

```c
void Disp0_DrawBitmap(int16_t x,
                      int16_t y,
                      int16_t width,
                      int16_t height,
                      const uint8_t *bitmap)
{
  /* PFB 模式：使用 DMA2D 把 tile 复制到帧缓冲 */
  uint32_t output_offset = __DISP0_CFG_SCEEN_WIDTH__ - width;
  uint32_t dst = (uint32_t)(&frame_buf[y * __DISP0_CFG_SCEEN_WIDTH__ + x]);

  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = output_offset;
  hdma2d.Init.LineOffsetMode = DMA2D_LOM_PIXELS;
  hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;
  hdma2d.Init.BytesSwap = DMA2D_BYTES_REGULAR;
  HAL_DMA2D_Init(&hdma2d);

  if (HAL_DMA2D_Start(&hdma2d,
                      (uint32_t)(bitmap),
                      dst,
                      width,
                      height) != HAL_OK)
  {
      dma2d_printf("Error, dma2d start failed\r\n");
  }

  if (HAL_DMA2D_PollForTransfer(&hdma2d, HAL_MAX_DELAY) != HAL_OK)
  {
    dma2d_printf("Error, transfer failed\r\n");
  }

#if defined (SCB_CleanDCache_by_Addr)
  {
    uint32_t len = (uint32_t)width * (uint32_t)height * 2U;
    SCB_CleanDCache_by_Addr((uint32_t *)dst, len);
  }
#endif
}
```

修正后，帧率提升到 91 FPS，LCD Latency 降到 1ms；但 CPU 占用升到 84.88%，需要继续优化。

### 异步刷新

打开 `__DISP0_CFG_ENABLE_ASYNC_FLUSHING__`，并使能 DMA2D 中断。

本工程异步刷新需要实现两件事：

1. LTDC Reload 中断通知 DMA2D：当前可在消隐窗口写 GRAM。
2. DMA2D 传输完成中断通知 ARM2D：本次发送完成，可继续渲染。

ARM2D 渲染完一个 tile 后，`IMPL_PFB_ON_LOW_LV_RENDERING()` 会尝试发送到 GRAM；若还没到消隐窗口，就需要等待。

工作流程如下：

```Plaintext
IMPL_PFB_ON_LOW_LV_RENDERING
-> __disp_adapter0_request_async_flushing
-> __disp0_try_start_pending_flush
```

在 `dma2d.c` 中实现了 `__disp_adapter0_request_async_flushing` 与 `__disp0_try_start_pending_flush`。

- `__disp_adapter0_request_async_flushing`：记录请求 + 置 pending + 尝试启动。
- `__disp0_try_start_pending_flush`：提交请求和 VBlank 到来两边都会调用；只有满足 `RR window open + pending + DMA idle` 才启动 DMA2D。

DMA2D 完成后的链路：

`DMA2D IRQ -> HAL -> __disp0_dma2d_xfer_cplt_cb / __disp0_dma2d_xfer_error_cb -> disp_adapter0_insert_async_flushing_complete_event_handler`

最终由该回调通知 Arm2D 本轮发送已完成。

LTDC 中断中的 `Disp0_OnVBlank()` 负责释放 `s_disp0_rr_window_open` 这把锁。

异步 + 1FB 测试结果：

- FPS 降到 66
- LCD Latency 约 70ms
- CPU 占用降到 18.48%

这会释放大量 CPU 资源，并降低画面撕裂风险。

#### 多个小 FB vs 一个大 FB

按当前逻辑推测，多个小 FB 的效率不如一个更大的 FB。瓶颈通常在 LTDC 一整屏传输时间；与其频繁切换多个小 FB，不如在扫描期间渲染更大的 tile。

测试数据：

| config      | FPS | LCD Latency | CPU loading |
| ----------- | --- | ----------- | ----------- |
| 2 800x48 FB | 66  | ~70ms       | 18.48%      |
| 1 800x96 FB | 76  | ~30ms       | 26.76%      |

> ⚠️ 修改这些配置后建议全量重编译，clean build 后性能通常会略有提升。

#### DMA2D 传输时间测量

目前 DMA2D 传输在消隐窗口内启动。按前面计算，LTDC 时钟 50MHz 时，消隐时间约 0.855ms。需要验证：

`LTDC reload 中断时间戳 -> DMA2D 传输完成中断时间戳 < 消隐时间`

在 LTDC 回调与 DMA2D 完成回调分别打点后，800x96 FB 的传输时间约为 0.14ms。

这个值不包含中断触发到软件打点语句（如 `LTDCStart = get_system_ticks();`）之间的前置开销，因此真实端到端耗时会稍高。由于结果远小于消隐时间，暂未继续深入修改 HAL 做更精确测量。

# 问题与 Bug

1. `.ARM.exidx` 有 ±1GB 距离限制，限制了内部 Flash（`0x0800_8000`）与外部 Flash（`0x9000_0000`）混放代码的链接。

   ```shell
   [build] ld.lld: error: <internal>:(.ARM.exidx+0x8): relocation R_ARM_PREL31 out of range: 1811987350 is not in [-1073741824, 1073741823]
   ```

   **临时方案：** 在链接脚本中丢弃 unwind 段（并非理想方案，需要谨慎评估）：

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

2. 在 STM32CubeMX 里勾选 DSP Library 后，代码生成并不会把预编译库（`arm_cortexM7l_math.a`）或 DSP 源文件复制到用户工程。

   该问题在 Windows 和 macOS 都出现过（测试版本 STM32CubeMX 6.16.0 / 6.17.0）。

   **解决方案：** 手工构建 CMSIS-DSP。可直接用 GitHub 源码，或从 STM32CubeMX 安装目录提取。

   > https://github.com/ARM-software/CMSIS-DSP

3. 外部 Flash 下载有时失败，会导致 hardfault。

   **解决方案：** 使用新的 SFL 代码。

4. VSCode 的 STM32Cube Build Analyzer（v1.1.0 / v1.2.0）对 ITCM（`0x0000_0000`）显示存在偏差。

   插件把 `.ARM.attributes`、`.symtab` 等不应计入的段也算进占用，导致显示值偏大；`.map` 文件里的 ITCM 占用是正确的。

   <img src="assets/image-20260201233005767.png" alt="image-20260201233005767" style="zoom: 67%;" /><img src="assets/image-20260201233026332.png" alt="image-20260201233026332" style="zoom:67%;" />

   **临时方案：** 等待插件更新。

5. LCD 运行一段时间后出现花屏。

   **解决方案：** LTDC 相关引脚输出速度必须配置为 **Very High**。

6. 下载完成后，程序可能进入 `MemFault()`。怀疑是 SFL 清理现场不完整导致。

   **解决方案：** 使用新的 SFL 代码。且在下载后强制复位，在 `launch.json` 添加 `preRunCommands`：

   ```json
   "preRunCommands": [
     "printf \"Reset MCU after download\\n\"",
     "monitor reset",
     "monitor sleep 100"
   ],
   ```

7. boot 有一定概率卡在 OSPI 初始化阶段。

   **解决方案：** 在 OSPI 初始化前先做一次反初始化，确保外设状态被正确复位。

# TODO

- [x] 外部 Flash Loader
- [x] Boot
- [x] Application
  - [x] UART 异步环形缓冲
    - [x] 发送使用软件环形缓冲（大数据量场景表现一般，后续按需优化）
    - [x] 接收使用 DMA 循环模式
  - [x] Button
  - [x] ARM2D
  - [x] Touch
