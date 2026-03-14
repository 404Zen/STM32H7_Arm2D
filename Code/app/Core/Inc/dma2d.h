/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dma2d.h
  * @brief   This file contains all the function prototypes for
  *          the dma2d.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DMA2D_H__
#define __DMA2D_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "async_uart.h"
/* USER CODE END Includes */

extern DMA2D_HandleTypeDef hdma2d;

/* USER CODE BEGIN Private defines */
#define DMA2D_DEBUG_ENABLE               1

#if DMA2D_DEBUG_ENABLE
    #define dma2d_printf(...)              debug_printf(__VA_ARGS__)
#else
    #define dma2d_printf(...)              (void)0
#endif
/* USER CODE END Private defines */

void MX_DMA2D_Init(void);

/* USER CODE BEGIN Prototypes */
// void DMA2D_Test(void);
void DMA2D_fill_screen(void);
void Disp0_DrawBitmap(int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t *bitmap);
void Disp0_OnVBlank(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __DMA2D_H__ */

