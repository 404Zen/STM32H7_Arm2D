/********************************************************************************************************
* @file     time_port.h
*
* @brief    Time process header file
*
* @author   404zen
*
* @date     2025-12-17 09:56:49
*
* @attention
*
* None.
*
*******************************************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIME_PORT_H__
#define __TIME_PORT_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define TIME_NOT_ARRIVED						0
#define TIME_IS_ARRIVED							1

uint32_t GetSysTime(void);
uint8_t SysTimeExceed(uint32_t start_ms, uint32_t time_ms);


#endif /* __TIME_PORT_H__ */
/*********************************END OF FILE**********************************/
