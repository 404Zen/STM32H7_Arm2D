/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dma2d.c
  * @brief   This file provides code for the configuration
  *          of the DMA2D instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "dma2d.h"

/* USER CODE BEGIN 0 */
#include "arm_2d_disp_adapter_0.h" 
#include <stdint.h>
#include "perf_counter.h"

#define FRAME_BUFFER_ADDR   ((uint16_t *)0x24044800U)
#define frame_buf           ((volatile uint16_t *)0x24044800)

/* Define the three frame buffer addresses for 3FB mode */
uintptr_t __DISP_ADAPTER0_3FB_FB0_ADDRESS__ = 0x24044800U;  /* 800*480*2 = 750KB */
uintptr_t __DISP_ADAPTER0_3FB_FB1_ADDRESS__ = 0x24044800U;  /* Same buffer (single-buffer mode) */
uintptr_t __DISP_ADAPTER0_3FB_FB2_ADDRESS__ = 0x24044800U;  /* Same buffer (single-buffer mode) */

static void __disp0_dma2d_xfer_cplt_cb(DMA2D_HandleTypeDef *hdma2d);
static void __disp0_dma2d_xfer_error_cb(DMA2D_HandleTypeDef *hdma2d);
static void __disp0_try_start_pending_flush(void);

typedef struct {
  int16_t iX;
  int16_t iY;
  int16_t iWidth;
  int16_t iHeight;
  const COLOUR_INT *pBuffer;
} disp0_async_flush_req_t;

static volatile bool s_disp0_rr_window_open;
static volatile bool s_disp0_async_pending;
static volatile bool s_disp0_async_busy;
static disp0_async_flush_req_t s_tDisp0AsyncReq;
/* USER CODE END 0 */

DMA2D_HandleTypeDef hdma2d;

/* DMA2D init function */
void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  hdma2d.LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */
  hdma2d.XferCpltCallback = __disp0_dma2d_xfer_cplt_cb;
  hdma2d.XferErrorCallback = __disp0_dma2d_xfer_error_cb;

  s_disp0_rr_window_open = false;
  s_disp0_async_pending = false;
  s_disp0_async_busy = false;
  
  /* Clear frame buffer at startup */
  {
      uint16_t *fb = FRAME_BUFFER_ADDR;
      uint32_t pixels = 800 * 480;
      for (uint32_t i = 0; i < pixels; i++) {
          fb[i] = 0x0000;  /* black */
      }
      SCB_CleanDCache_by_Addr((uint32_t *)fb, pixels * 2);
  }
  
  /* USER CODE END DMA2D_Init 2 */

}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* dma2dHandle)
{

  if(dma2dHandle->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspInit 0 */

  /* USER CODE END DMA2D_MspInit 0 */
    /* DMA2D clock enable */
    __HAL_RCC_DMA2D_CLK_ENABLE();

    /* DMA2D interrupt Init */
    HAL_NVIC_SetPriority(DMA2D_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2D_IRQn);
  /* USER CODE BEGIN DMA2D_MspInit 1 */

  /* USER CODE END DMA2D_MspInit 1 */
  }
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* dma2dHandle)
{

  if(dma2dHandle->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspDeInit 0 */

  /* USER CODE END DMA2D_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DMA2D_CLK_DISABLE();

    /* DMA2D interrupt Deinit */
    HAL_NVIC_DisableIRQ(DMA2D_IRQn);
  /* USER CODE BEGIN DMA2D_MspDeInit 1 */

  /* USER CODE END DMA2D_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
#if 0
uint16_t __attribute__((section(".sram_bss"))) test_buf[200*100];
void DMA2D_Test(void)
{
  int screen_width = 800;
  int rect_width = 200;
  int rect_height = 100;

  for(uint32_t i = 0; i < rect_width * rect_height; i++)
  {
    test_buf[i] = 0xF800; // Red color in RGB565
  }

#if 1
  GLCD_DrawBitmap(20, 50, 100, 50, (const uint8_t *)test_buf);
#else
  // 在800×480屏幕上更新200×100的矩形，起始位置(100,50)
  


  
  hdma2d.Init.Mode = DMA2D_M2M;  // 内存到内存模式
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = screen_width - rect_width;  // 输出行偏移设置为屏幕宽度减去矩形宽度
  hdma2d.Init.LineOffsetMode = DMA2D_LOM_PIXELS; // 行偏移以像素为单位
  hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  // Alpha不反转
  hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;       // 不交换红蓝
  hdma2d.Init.BytesSwap = DMA2D_BYTES_REGULAR;      // 不交换字节
  HAL_DMA2D_Init(&hdma2d);

  // hdma2d.LayerCfg[0].InputOffset = 0;  // 源图像没有行间隔
  // hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;  // 源格式
  // hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;     // 不修改Alpha
  // hdma2d.LayerCfg[0].InputAlpha = 0xFF;                    // 不透明
  // HAL_DMA2D_ConfigLayer(&hdma2d, 0); // 配置



  if (HAL_DMA2D_Start(&hdma2d,
                        (uint32_t)test_buf,
                        (uint32_t)(&frame_buf[50 * screen_width + 100]), // 计算目标地址
                        rect_width,
                        rect_height) != HAL_OK)
    {
        dma2d_printf("Error, dma2d start failed\r\n");
    }

  if(HAL_DMA2D_PollForTransfer(&hdma2d, HAL_MAX_DELAY) != HAL_OK)
  {
    dma2d_printf("Error, transfer failed\r\n");
  }
#endif
}
#endif

void DMA2D_fill_screen(void)
{
#if 1
  /* Fill GRAM using CPU */
  uint32_t  i;
    volatile uint16_t *ptr_frame_buf;

    ptr_frame_buf = frame_buf;
    for (i = 0; i < (800  * 480); i++) {
        *ptr_frame_buf++ = 0x001F;
    }
#else
	DMA2D->CR	  &=	~(DMA2D_CR_START);				//	停止DMA2D
	DMA2D->CR		=	DMA2D_R2M;							//	寄存器到SDRAM
	DMA2D->OPFCCR	=	LTDC_PIXEL_FORMAT_RGB565;						//	设置颜色格式
	DMA2D->OOR		=	0;										//	设置行偏移 
  DMA2D->OMAR		=	0x24044800 ;				// 地址
	DMA2D->NLR		=	(800<<16)|(480);	//	设定长度和宽度
	DMA2D->OCOLR	=	0xF100;						//	颜色
	
/******	
等待 垂直数据使能显示状态 ，即LTDC即将刷完一整屏数据的时候
因为在屏幕没有刷完一帧时进行刷屏，会有撕裂的现象
用户也可以使用 寄存器重载中断 进行判断，不过为了保证例程的简洁以及移植的方便性，这里直接使用判断寄存器的方法
	
如果不做垂直等待判断，DMA2D刷屏速度为：颜色格式	RGB565	1.4ms	(712帧)

加了垂直等待之后，需要9ms刷一屏，不过屏幕本身的刷新率只有60帧左右（LTDC时钟33MHz），
实际9ms的速度已经足够了，除非是对速度要求特别高的场合，不然建议加上判断垂直等待的语句，可以避免撕裂效应

******/
	while( LTDC->CDSR != 0X00000001);	// 判断 显示状态寄存器LTDC_CDSR 的第0位 VDES：垂直数据使能显示状态
	
	DMA2D->CR	  |=	DMA2D_CR_START;					//	启动DMA2D
		
	while (DMA2D->CR & DMA2D_CR_START) ;				//	等待传输完成
#endif
}



__OVERRIDE_WEAK
void Disp0_DrawBitmap(int16_t x,
                      int16_t y,
                      int16_t width,
                      int16_t height,
                      const uint8_t *bitmap)
{
  /* In 3FB mode, ARM2D writes directly to frame buffer - this should not be called */
#if __DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__
  (void)x; (void)y; (void)width; (void)height; (void)bitmap;
#else
  /* PFB mode: copy tile to frame buffer using DMA2D */
  uint32_t output_offset = __DISP0_CFG_SCEEN_WIDTH__ - width; // 计算行偏移，单位为像素
  //uint32_t dst = &frame_buf[y * __DISP0_CFG_SCEEN_WIDTH__ + x];

  hdma2d.Init.Mode = DMA2D_M2M;  // 内存到内存模式
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = output_offset;  // 输出行偏移设置为屏幕宽度减去矩形宽度
  hdma2d.Init.LineOffsetMode = DMA2D_LOM_PIXELS; // 行偏移以像素为单位
  hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  // Alpha不反转
  hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;       // 不交换红蓝
  hdma2d.Init.BytesSwap = DMA2D_BYTES_REGULAR;      // 不交换字节
  HAL_DMA2D_Init(&hdma2d);

  if (HAL_DMA2D_Start(&hdma2d,
                      (uint32_t)(bitmap),
                      (uint32_t)(&frame_buf[y * __DISP0_CFG_SCEEN_WIDTH__ + x]), // 计算目标地址
                      width,
                      height) != HAL_OK)
  {
      dma2d_printf("Error, dma2d start failed\r\n");
  }

  if(HAL_DMA2D_PollForTransfer(&hdma2d, HAL_MAX_DELAY) != HAL_OK)
  {
    dma2d_printf("Error, transfer failed\r\n");
  }

  #if defined (SCB_CleanDCache_by_Addr)
  {
    uint32_t len = (uint32_t)width * (uint32_t)height * 2U;
    SCB_CleanDCache_by_Addr((uint32_t *)dst, len);
  }
  #endif

#endif
}

void __disp_adapter0_request_async_flushing(   
    void *pTarget,  
    bool bIsNewFrame,  
    int16_t iX,   
    int16_t iY,  
    int16_t iWidth,  
    int16_t iHeight,  
    const COLOUR_INT *pBuffer)
{
  ARM_2D_UNUSED(pTarget);
  ARM_2D_UNUSED(bIsNewFrame);

  s_tDisp0AsyncReq.iX = iX;
  s_tDisp0AsyncReq.iY = iY;
  s_tDisp0AsyncReq.iWidth = iWidth;
  s_tDisp0AsyncReq.iHeight = iHeight;
  s_tDisp0AsyncReq.pBuffer = pBuffer;
  s_disp0_async_pending = true;

  __disp0_try_start_pending_flush();
}

void Disp0_OnVBlank(void)
{
#if __DISP0_CFG_ENABLE_ASYNC_FLUSHING__
  s_disp0_rr_window_open = true;
  __disp0_try_start_pending_flush();
#endif
}

static void __disp0_try_start_pending_flush(void)
{
#if __DISP0_CFG_ENABLE_ASYNC_FLUSHING__
  uint32_t output_offset;
  disp0_async_flush_req_t tReq;

  if ((!s_disp0_rr_window_open) || (!s_disp0_async_pending) || s_disp0_async_busy) {
    return;
  }

  tReq = s_tDisp0AsyncReq;
  s_disp0_async_pending = false;
  s_disp0_rr_window_open = false;
  s_disp0_async_busy = true;

  output_offset = __DISP0_CFG_SCEEN_WIDTH__ - tReq.iWidth;

  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = output_offset;
  hdma2d.Init.LineOffsetMode = DMA2D_LOM_PIXELS;
  hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;
  hdma2d.Init.BytesSwap = DMA2D_BYTES_REGULAR;
  HAL_DMA2D_Init(&hdma2d);

  if (HAL_DMA2D_Start_IT(&hdma2d,
                        (uint32_t)(tReq.pBuffer),
                        (uint32_t)(&frame_buf[tReq.iY * __DISP0_CFG_SCEEN_WIDTH__ + tReq.iX]),
                        tReq.iWidth,
                        tReq.iHeight) != HAL_OK)
  {
      s_disp0_async_busy = false;
      dma2d_printf("Error, dma2d start failed\r\n");
      disp_adapter0_insert_async_flushing_complete_event_handler();
  }
#endif
}

extern int64_t LTDCStart;
int64_t DMA2DTransTick = 0;
static void __disp0_dma2d_xfer_cplt_cb(DMA2D_HandleTypeDef *hdma2d)
{
  (void)hdma2d;
  s_disp0_async_busy = false;
#if __DISP0_CFG_ENABLE_ASYNC_FLUSHING__
  DMA2DTransTick = get_system_ticks() - LTDCStart;
  disp_adapter0_insert_async_flushing_complete_event_handler();
#endif
}

static void __disp0_dma2d_xfer_error_cb(DMA2D_HandleTypeDef *hdma2d)
{
  (void)hdma2d;
  s_disp0_async_busy = false;
#if __DISP0_CFG_ENABLE_ASYNC_FLUSHING__
  dma2d_printf("Error, dma2d transfer error\r\n");
  disp_adapter0_insert_async_flushing_complete_event_handler();
#endif
}
/* USER CODE END 1 */

