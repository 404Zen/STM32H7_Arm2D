/**
 * @file GT9xx_touch.h
 * @brief Brief description of the header file
 * @author 404zen
 * @date 2026-03-01
 * @version 1.0
 */

#ifndef __GT9XX_TOUCH_H__
#define __GT9XX_TOUCH_H__

#include "io_i2c.h"
#include "async_uart.h"
#include <stdint.h>
#include "main.h"

#define GT9XX_DEBUG_ENABLE               1

#if GT9XX_DEBUG_ENABLE
    #define GT9XX_printf(...)              debug_printf(__VA_ARGS__)
#else
    #define GT9XX_printf(...)              (void)0
#endif

#define GT9XX_I2C_TIMEOUT_MS              1000

#define GT9XX_MAX_TOUCH_POINTS          5

#define GT9XX_DEVICE_ADDR               0xBA

#define GT9XX_REG_ID_ADDR               0x8140          /* 11 bytes ID and resolution data */
#define GT9XX_REG_CFG_VER_ADDR          0x8047          /* 1 byte config version */
#define GT9XX_REG_COORD_ADDR            0x814E          /* 30 bytes coordinate data for 5 points */


typedef enum
{
    GT9XX_M_STATE_PWR_ON = 0,
    GT9XX_M_STATE_DEV_ADDR_SET0,
    GT9XX_M_STATE_DEV_ADDR_SET1,
    GT9XX_M_STATE_DEV_ADDR_SET2,

    GT9XX_M_STATE_READ_ID_CMD,
    GT9XX_M_STATE_READ_ID,

    GT9XX_M_STATE_READ_CFG_CMD,
    GT9XX_M_STATE_READ_CFG,

    GT9XX_M_STATE_READ_COORD_GAP,
    GT9XX_M_STATE_READ_COORD_CMD,
    GT9XX_M_STATE_READ_COORD,

    GT9XX_M_STATE_COORD_CLR_CMD,
    GT9XX_M_STATE_COORD_CLR,

    GT9XX_M_STATE_ERROR_LOG,
    GT9XX_M_STATE_ERROR,
    
}GT9XX_M_State_t;


typedef struct __attribute__((packed))
{
    uint8_t track_id;
    uint16_t x;
    uint16_t y;
    uint16_t size;
    uint8_t reserved;
}GT9XX_TouchPoint_t;

/* 1byte aligned */
typedef struct __attribute__((packed))
{
    uint8_t status;
    GT9XX_TouchPoint_t points[GT9XX_MAX_TOUCH_POINTS];
    uint8_t reserved;
}GT9XX_TouchData_t;

typedef struct
{
    GT9XX_M_State_t gt9xx_m_state;
    uint8_t touch_state;
    GT9XX_TouchData_t t_raw_data;                       /* touch data read from device */
    
    uint8_t vaild_points_num;
    GT9XX_TouchPoint_t touch_points[GT9XX_MAX_TOUCH_POINTS];    /* touch data copied from raw data */

    uint8_t cfg_ver;
    uint8_t touch_id[11];                       /* 11 bytes ID and resolution data */
    void *i2c_instance;
    uint8_t i2c_bus_lock;                       /* 0-idle 1-busy */
}GT9XX_Touch_Handle_t;

void GT9XX_Touch_Init(void);
void GT911_LoopTask(void);

#endif /* __GT9XX_TOUCH_H__ */
