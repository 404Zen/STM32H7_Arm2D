/**
 * @file GT9xx_touch.c
 * @brief Brief description of the source file
 * @author 404zen
 * @date 2026-03-01
 * @version 1.0
 */

#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include "GT9xx_touch.h"
#include "cmsis_gcc.h"
#include "time_port.h"
#include "data_conversion.h"



struct io_i2c_instance_t GT911_i2c;
GT9XX_Touch_Handle_t GT911 = {0};


void GT911_i2c_callback(uint8_t event)
{
    
    if(event == 0)
    {
        GT911.i2c_bus_lock = 0;
    }
    else 
    {
        GT9XX_printf("GT9XX_i2c Error, event = %d\r\n", event);
    }
}

static void GT911_INT_InFloating_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = TOUCH_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(TOUCH_INT_GPIO_Port, &GPIO_InitStruct);
}

static int GT9XX_I2C_Read(GT9XX_Touch_Handle_t *touch_handle, uint16_t reg_addr, uint16_t len, uint8_t *data)
{
    touch_handle->i2c_bus_lock = 1;
    int result = io_i2c_read_reg(touch_handle->i2c_instance, reg_addr, len, data);
    return result;
}

static int GT9XX_I2C_Write(GT9XX_Touch_Handle_t *touch_handle, uint16_t reg_addr, uint16_t len, uint8_t *data)
{
    touch_handle->i2c_bus_lock = 1;
    int result = io_i2c_write_reg(touch_handle->i2c_instance, reg_addr, len, data);
    return result;
}

void GT9XX_Touch_Init(void)
{
    GT911_i2c.instance_id = 0;

    GT911_i2c.scl_port = TOUCH_SCL_GPIO_Port;
    GT911_i2c.scl_pin = TOUCH_SCL_Pin;
    GT911_i2c.sda_port = TOUCH_SDA_GPIO_Port;
    GT911_i2c.sda_pin = TOUCH_SDA_Pin;

    GT911_i2c.device_addr = GT9XX_DEVICE_ADDR;
    GT911_i2c.device_addr_mode = 0;
    GT911_i2c.callback = GT911_i2c_callback;
    GT911_i2c.reg_address_bytes = 2;

    GT911_i2c.speed = 100;

    io_i2c_instance_register(&GT911_i2c);

    GT911.i2c_instance = &GT911_i2c;
    GT911.i2c_bus_lock = 0;

    GT911.vaild_points_num = 0;

    GT911.gt9xx_m_state = GT9XX_M_STATE_PWR_ON;
}


void GT911_LoopTask(void)
{
    static uint32_t start_timer = 0;
    uint32_t i = 0;

    switch (GT911.gt9xx_m_state) 
    {
        case GT9XX_M_STATE_PWR_ON:
            GT911.gt9xx_m_state = GT9XX_M_STATE_DEV_ADDR_SET0;
            break;
        
        case GT9XX_M_STATE_DEV_ADDR_SET0:
            GT9XX_printf("GT9XX_M_STATE_DEV_ADDR_SET\r\n");
            /* TOUCH_INT_Pin is init as output push pull before */
            /* Pull up INT, keep 100us and then pull up RESET, device address set to 0x29 */
            /* Keep INT low,keep 100us and then pull up RESET, device address set to 0xBA */
            HAL_GPIO_WritePin(TOUCH_INT_GPIO_Port, TOUCH_INT_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(TOUCH_RESET_GPIO_Port, TOUCH_RESET_Pin, GPIO_PIN_RESET);
            
            start_timer = GetSysTime();
            GT911.gt9xx_m_state = GT9XX_M_STATE_DEV_ADDR_SET1;
            break;
        
        case GT9XX_M_STATE_DEV_ADDR_SET1:
            if(SysTimeExceed(start_timer, 30) == TIME_IS_ARRIVED)
            {
                HAL_GPIO_WritePin(TOUCH_RESET_GPIO_Port, TOUCH_RESET_Pin, GPIO_PIN_SET);

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_DEV_ADDR_SET2;
            }
            break;

        case GT9XX_M_STATE_DEV_ADDR_SET2:
            if(SysTimeExceed(start_timer, 20) == TIME_IS_ARRIVED)
            {
                /* 5ms after RESET pull up , INT pin set to floating input */
                GT911_INT_InFloating_Init();

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_READ_ID_CMD;
            }
            break;

        
        

        case GT9XX_M_STATE_READ_ID_CMD:
            if(GT911.i2c_bus_lock == 0)
            {
                GT9XX_printf("GT9XX_M_STATE_READ_ID_CMD\r\n");
                GT9XX_I2C_Read(&GT911, GT9XX_REG_ID_ADDR, 11, GT911.touch_id);

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_READ_ID;
            }
            else if(SysTimeExceed(start_timer, GT9XX_I2C_TIMEOUT_MS) == TIME_IS_ARRIVED)
            {
                GT9XX_printf("GT9XX_M_STATE_READ_ID_CMD timeout, reset device!!!\r\n");

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_PWR_ON;
            }
            break;

        case GT9XX_M_STATE_READ_ID:
            if(GT911.i2c_bus_lock == 0)
            {
                GT9XX_printf("Touch ID is GT%c%c%c\r\n", GT911.touch_id[0], GT911.touch_id[1], GT911.touch_id[2]);
                GT9XX_printf("FW Version : 0x%04X\r\n", (GT911.touch_id[5] << 8) | GT911.touch_id[4]);
                GT9XX_printf("Resloution : X = %d, Y = %d.\r\n", (GT911.touch_id[7] << 8) | GT911.touch_id[6], (GT911.touch_id[9] << 8) | GT911.touch_id[8]);

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_READ_CFG_CMD;
            }
            else if(SysTimeExceed(start_timer, GT9XX_I2C_TIMEOUT_MS) == TIME_IS_ARRIVED)
            {
                GT9XX_printf("GT9XX_M_STATE_READ_ID timeout, reset device!!!\r\n");

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_PWR_ON;
            }
            break;

        case GT9XX_M_STATE_READ_CFG_CMD:
            if(GT911.i2c_bus_lock == 0)
            {
                GT9XX_printf("GT9XX_M_STATE_READ_CFG_CMD\r\n");
                GT9XX_I2C_Read(&GT911, GT9XX_REG_CFG_VER_ADDR, 1, &GT911.cfg_ver);
                

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_READ_CFG;
            }
            else if(SysTimeExceed(start_timer, GT9XX_I2C_TIMEOUT_MS) == TIME_IS_ARRIVED)
            {
                GT9XX_printf("GT9XX_M_STATE_READ_CFG_CMD timeout, reset device!!!\r\n");

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_PWR_ON;
            }
            break;

        case GT9XX_M_STATE_READ_CFG:
            if(GT911.i2c_bus_lock == 0)
            {
                GT9XX_printf("Config version: 0x%02X\r\n", GT911.cfg_ver);

                start_timer = GetSysTime();
                // GT911.gt9xx_m_state = GT9XX_M_STATE_READ_COORD_CMD;
                GT911.gt9xx_m_state = GT9XX_M_STATE_COORD_CLR_CMD;
            }
            else if(SysTimeExceed(start_timer, GT9XX_I2C_TIMEOUT_MS) == TIME_IS_ARRIVED)
            {
                GT9XX_printf("GT9XX_M_STATE_READ_CFG timeout, reset device!!!\r\n");

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_PWR_ON;
            }
            break;

        

        
        case GT9XX_M_STATE_READ_COORD_CMD:
            if(GT911.i2c_bus_lock == 0)
            {
                GT9XX_I2C_Read(&GT911, GT9XX_REG_COORD_ADDR, (2+8*GT9XX_MAX_TOUCH_POINTS), (uint8_t *)&(GT911.t_raw_data));

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_READ_COORD;
            }
            else if(SysTimeExceed(start_timer, GT9XX_I2C_TIMEOUT_MS) == TIME_IS_ARRIVED)
            {
                GT9XX_printf("GT9XX_M_STATE_READ_COORD_CMD timeout, reset device!!!\r\n");

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_PWR_ON;
            }
             break;
        
        case GT9XX_M_STATE_READ_COORD:
            if(GT911.i2c_bus_lock == 0)
            {
                GT911.vaild_points_num = GT911.t_raw_data.status & 0x0F;

                if(( GT911.vaild_points_num >= 1 ) && ( GT911.vaild_points_num <= GT9XX_MAX_TOUCH_POINTS ))
                {

                    GT9XX_printf("Touch points: %d\r\n", GT911.vaild_points_num);

                    for(i = 0; i < GT911.vaild_points_num; i++)
                    {
                        GT911.touch_points[i].track_id = GT911.t_raw_data.points[i].track_id;
                        GT911.touch_points[i].x = (GT911.t_raw_data.points[i].x);
                        GT911.touch_points[i].y = (GT911.t_raw_data.points[i].y);
                        GT911.touch_points[i].size = (GT911.t_raw_data.points[i].size);
                        
                        GT9XX_printf("ID = %d, X = %d, Y = %d, Size = %d\r\n", GT911.touch_points[i].track_id, GT911.touch_points[i].x, GT911.touch_points[i].y, GT911.touch_points[i].size);

                        if(GT911.touch_points[i].x > 800)
                        {
                            __BKPT(0);
                        }
                    }
                }
                else
                {
                    GT911.vaild_points_num = 0;
                    // GT9XX_printf("No touch detected\r\n");
                }

                if((GT911.t_raw_data.status & 0x40) == 0x40)
                {
                    GT9XX_printf("GT9XX large detect!!!\r\n");
                    GT911.gt9xx_m_state = GT9XX_M_STATE_ERROR;
                }
                else 
                {
                    start_timer = GetSysTime();
                    GT911.gt9xx_m_state = GT9XX_M_STATE_COORD_CLR_CMD;
                }
            }
            else if(SysTimeExceed(start_timer, GT9XX_I2C_TIMEOUT_MS) == TIME_IS_ARRIVED)
            {
                GT9XX_printf("GT9XX_M_STATE_READ_COORD timeout, reset device!!!\r\n");

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_PWR_ON;
            }
            break;

        case GT9XX_M_STATE_COORD_CLR_CMD:
            if(GT911.i2c_bus_lock == 0)
            {
                uint8_t clr_data = 0x00;
                GT9XX_I2C_Write(&GT911, GT9XX_REG_COORD_ADDR, 1, &clr_data);

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_READ_COORD_GAP;
            }
            else if(SysTimeExceed(start_timer, GT9XX_I2C_TIMEOUT_MS) == TIME_IS_ARRIVED)
            {
                GT9XX_printf("GT9XX_M_STATE_COORD_CLR_CMD timeout, reset device!!!\r\n");

                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_PWR_ON;
            }
            break;

        case GT9XX_M_STATE_READ_COORD_GAP:
            /* max speed is 100Hz, 10ms */
            if(SysTimeExceed(start_timer, 20) == TIME_IS_ARRIVED)
            {
                start_timer = GetSysTime();
                GT911.gt9xx_m_state = GT9XX_M_STATE_READ_COORD_CMD;
            }
            break;
        
        case GT9XX_M_STATE_ERROR_LOG:
            GT9XX_printf("GT9XX Error, Touch will not work!!!\r\n");
            break;

        case GT9XX_M_STATE_ERROR:
            break;
        

        default:
            break;
    
    }
}
