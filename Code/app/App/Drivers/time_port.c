/********************************************************************************************************
* @file     time_port.c
*
* @brief    Time process source file
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

/* Includes ------------------------------------------------------------------*/
#include "time_port.h"



uint32_t GetSysTime(void)
{
	return HAL_GetTick();
}

uint8_t SysTimeExceed(uint32_t start_ms, uint32_t time_ms)
{
	uint32_t time_now = 0;

	time_now = HAL_GetTick();

	if(time_now >= start_ms)
	{
		if(time_now - start_ms >= time_ms)
		{
			return TIME_IS_ARRIVED;
		}
	}
	else
	{
		if((HAL_MAX_DELAY - start_ms) + time_now >= time_ms)
		{
			return TIME_IS_ARRIVED;
		}
	}

	return TIME_NOT_ARRIVED;
}

