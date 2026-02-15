/**
 * @file io_i2c.c
 * @brief Brief description of the source file
 * @author 404zen
 * @date 2026-02-10
 * @version 1.0
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "io_i2c.h"

//Platform
#include "stm32h7xx_hal_gpio.h"
#include "async_uart.h"

extern async_uart_instance_t uart1;
static io_i2c_platform_t platform_ops;

struct io_i2c_instance_t touch_i2c;
static struct io_i2c_instance_t *loop_list = NULL;

void test_callback(uint8_t event);

static int io_i2c_time_exceed(uint32_t *start_time, uint32_t *time_out);
static int io_i2c_state_machine_read(struct io_i2c_instance_t *instance);

void io_i2c_init(void)
{
    /* Platform init */
    platform_ops.io_i2c_pin_write = io_i2c_write;
    platform_ops.io_i2c_pin_read = io_i2c_read_adapter;
    platform_ops.io_i2c_get_tick = io_i2c_get_tick;
    platform_ops.io_i2c_reload_tick = user_systick_reload_get() + 1;            // 280000 = 1ms
    

    /* Instance init */
    touch_i2c.instance_id = 0;
    touch_i2c.scl_port = TOUCH_SCL_GPIO_Port;
    touch_i2c.scl_pin = TOUCH_SCL_Pin;
    touch_i2c.sda_port = TOUCH_SDA_GPIO_Port;
    touch_i2c.sda_pin = TOUCH_SDA_Pin;

    touch_i2c.device_addr = 0xBA;
    touch_i2c.device_addr_mode = 0;
    touch_i2c.callback = test_callback;

    touch_i2c.speed = 100;

    io_i2c_instance_register(&touch_i2c);
    platform_ops.io_i2c_pin_write(touch_i2c.sda_port, touch_i2c.scl_pin, 1);
    platform_ops.io_i2c_pin_write(touch_i2c.scl_port, touch_i2c.scl_pin, 1);
}

int io_i2c_instance_register(struct io_i2c_instance_t *instance)
{
    uint32_t sys_core_clk = 0;
    if(instance == NULL)
    {
        return -1;
    }

    while(loop_list != NULL)
    {
        loop_list = loop_list->next;
    }

    loop_list = instance;

    loop_list->state = IO_I2C_IDLE;
    loop_list->ops_param.ops_state = IO_I2C_OPS_IDLE;
    loop_list->ops_param.reg_addr = 0;
    loop_list->ops_param.data_len = 0;
    loop_list->ops_param.data = NULL;
    loop_list->ops_param.sending_data = 0;
    loop_list->ops_param.sending_bit = 0;
    loop_list->ops_param.sending_bytes = 0;
    loop_list->ops_param.start_tick = 0;
    loop_list->ops_param.exceed_tick = 0;
    sys_core_clk = user_get_system_core_clk();
    /* 100 KHz = 10 us = 2800 ticks */ 
    loop_list->ops_param.delay_tick = sys_core_clk/1000/loop_list->speed/2;


    loop_list->next = NULL;

    return 0;
}

void io_i2c_loop_task(void)
{
    struct io_i2c_instance_t *list = loop_list;

    while(list != NULL)
    {
        switch (list->state) 
        {
            case IO_I2C_IDLE:
                
                break;

            case IO_I2C_READ:
                io_i2c_state_machine_read(list);
                break;

            case IO_I2C_WRITE:
                break;
                
            case IO_I2C_ERROR:
                break;

            default:
                break;
        }

        list = list->next;
    }
}

static int io_i2c_time_exceed(uint32_t *start_time, uint32_t *time_out)
{
    uint32_t temp_tick = platform_ops.io_i2c_get_tick();
    uint32_t esc_tick = 0;
    /* systick is a count down counter */ 
    if(temp_tick < *start_time) 
    {
        esc_tick = *start_time - temp_tick;
    }
    else 
    {
        esc_tick = *start_time + (platform_ops.io_i2c_reload_tick - temp_tick);
    }

    *start_time = temp_tick;
    if(esc_tick >= *time_out)
    {
        *time_out = 0;
    }
    else 
    {
        *time_out -= esc_tick;
    }

    /* return remain time */
    return *time_out;
}

static int io_i2c_state_machine_read(struct io_i2c_instance_t *instance)
{
    static uint32_t sda_val = 0;
    switch (instance->ops_param.ops_state) 
    {
        /* start signal begin */
        case IO_I2C_OPS_IDLE:
            /* pull up scl and sda prepare for start  */ 
            platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,1);
            platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,1);
            instance->ops_param.start_tick = platform_ops.io_i2c_get_tick();
            instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
            instance->ops_param.ops_state = IO_I2C_OPS_START_SDA0;
            break;
        
        case IO_I2C_OPS_START_SDA0:
        case IO_I2C_OPS_SR_SDA0:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,0);
                /* no need get new start tick ,it is updated in io_i2c_time_exceed() */ 
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;   
                
                if(instance->ops_param.ops_state == IO_I2C_OPS_START_SDA0)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_START_SCL0;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_SR_SDA0)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_SR_SCL0;
                }
                
            }
            break;

        case IO_I2C_OPS_START_SCL0:
        case IO_I2C_OPS_SR_SCL0:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,0);
                /* no need get new start tick ,it is updated in io_i2c_time_exceed() */ 
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;

                if(instance->ops_param.ops_state  == IO_I2C_OPS_START_SCL0)
                {
                    /* next time send device address MSB */ 
                    /* 7 bits address */ 
                    if(instance->device_addr_mode == 0)
                    {
                        instance->ops_param.sending_data = instance->device_addr;
                        instance->ops_param.sending_bytes = 1;
                    }
                    /* this case IS NOT TESTED */
                    /* 10 bits address  */ 
                    else 
                    {   
                        instance->ops_param.sending_data = (instance->device_addr >> 8);
                        instance->ops_param.sending_bytes = 2;
                    }
                    instance->ops_param.sending_bit = 8;

                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_SDA_SET;
                }

                if(instance->ops_param.ops_state  == IO_I2C_OPS_SR_SCL0)
                {
                    /* Read data*/
                    /* 7 bits address */ 
                    if(instance->device_addr_mode == 0)
                    {
                        instance->ops_param.sending_data = (instance->device_addr | 0x01);
                        instance->ops_param.sending_bytes = 1;
                    }
                    else 
                    {
                        instance->ops_param.sending_data = ((instance->device_addr >> 8) | 0x01);
                        instance->ops_param.sending_bytes = 2;
                    }
                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_RADDR_SDA_SET;
                }

            }
            break;
        /* start signal end */

        /* device address msb begin and loop  */
        /* reg address begin and loop */
        /* SDA = data， SCL = 0 */
        case IO_I2C_OPS_DEV_ADDR_SDA_SET:
        case IO_I2C_OPS_REG_ADDR_SDA_SET:
        case IO_I2C_OPS_DATA_SDA_SET:
        case IO_I2C_OPS_DEV_RADDR_SDA_SET:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                /* no need get new start tick ,it is updated in io_i2c_time_exceed() */ 
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                /* set sda line according device address */
                if(--(instance->ops_param.sending_bit))
                {
                    sda_val =((instance->ops_param.sending_data & (0x01 << instance->ops_param.sending_bit)) == 0x00) ? 0 : 1;
                    platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,sda_val);
                    
                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_ADDR_SDA_SET)
                    {   
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_SCL_SET;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_SDA_SET)
                    {
                        instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_SCL_SET;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DATA_SDA_SET)
                    {
                        instance->ops_param.ops_state = IO_I2C_OPS_DATA_SCL_SET;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_SDA_SET)
                    {
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_RADDR_SCL_SET;
                    }
                }
                else 
                {
                    /* next is ack, pull up sda and hope device pull it down */
                    platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,1);
                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_ADDR_SDA_SET)
                    {
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_ACK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_SDA_SET)
                    {
                        instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_ACK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DATA_SDA_SET)
                    {
                        instance->ops_param.ops_state = IO_I2C_OPS_DATA_ACK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_SDA_SET)
                    {
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_RADDR_ACK;
                    }
                }

            }
            break;

        /* SDA = data， SCL = 1, data vaild time set */
        case IO_I2C_OPS_DEV_ADDR_SCL_SET:
        case IO_I2C_OPS_REG_ADDR_SCL_SET:
        case IO_I2C_OPS_DATA_SCL_SET:
        case IO_I2C_OPS_DEV_RADDR_SCL_SET:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                /* no need get new start tick ,it is updated in io_i2c_time_exceed() */ 
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,1);

                if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_ADDR_SCL_SET)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_SCL_RESET;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_SCL_SET)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_SCL_RESET;
                }
                
                if(instance->ops_param.ops_state == IO_I2C_OPS_DATA_SCL_SET)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_DATA_SCL_RESET;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_SCL_SET)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_RADDR_SCL_RESET;
                }
            }
            break;
        
        /* SDA = data， SCL = 1->0, go to prepare next data */
        case IO_I2C_OPS_DEV_ADDR_SCL_RESET:
        case IO_I2C_OPS_REG_ADDR_SCL_RESET:
        case IO_I2C_OPS_DATA_SCL_RESET:
        case IO_I2C_OPS_DEV_RADDR_SCL_RESET:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                /* no need get new start tick ,it is updated in io_i2c_time_exceed() */ 
                instance->ops_param.exceed_tick = 0;
                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,0);

                if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_ADDR_SCL_RESET)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_SDA_SET;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_SCL_RESET)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_SDA_SET;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_DATA_SCL_RESET)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_DATA_SDA_SET;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_SCL_RESET)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_RADDR_SDA_SET;
                }
            }
            break;

        /* SDA = 1, SCL = 0->1, gen clock for ack */
        case IO_I2C_OPS_DEV_ADDR_ACK:
        case IO_I2C_OPS_REG_ADDR_ACK:
        case IO_I2C_OPS_DATA_ACK:
        case IO_I2C_OPS_DEV_RADDR_ACK:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                /* no need get new start tick ,it is updated in io_i2c_time_exceed() */ 
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,1);

                if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_ADDR_ACK)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_ACK_READ;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_ACK)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_ACK_READ;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_ACK)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_RADDR_ACK_READ;
                }
            }
            break;

        /* sda = ack, scl = 1->0, read ack and the pull down scl */
        case IO_I2C_OPS_DEV_ADDR_ACK_READ:
        case IO_I2C_OPS_REG_ADDR_ACK_READ:
        case IO_I2C_OPS_DEV_RADDR_ACK_READ:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                /* no need get new start tick ,it is updated in io_i2c_time_exceed() */ 
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;

                if(platform_ops.io_i2c_pin_read(instance->sda_port, instance->sda_pin) != 0)
                {
                    /* device no ack */
                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_ACK_FAILED;
                }
                else 
                {
                    /* device ack */
                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_ADDR_ACK_READ)
                    {   
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_ACK_OK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_ACK_READ)
                    {   
                        instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_ACK_OK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_ACK_READ)
                    {   
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_RADDR_ACK_OK;
                    }
                }

                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,0);
            }
            break;

        /* SDA = 1， SCL = 0 */
        case IO_I2C_OPS_DEV_ADDR_ACK_OK:
        case IO_I2C_OPS_REG_ADDR_ACK_OK:
        case IO_I2C_OPS_DEV_RADDR_ACK_OK:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                /* no need get new start tick ,it is updated in io_i2c_time_exceed() */ 
                instance->ops_param.exceed_tick = 0;
                instance->ops_param.sending_bytes--;

                /* data unfinished in this phase */
                if(instance->ops_param.sending_bytes)
                {
                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_ADDR_ACK_OK)
                    {
                        instance->ops_param.sending_data = instance->device_addr & 0xFF;
                        instance->ops_param.sending_bit = 8;
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_SDA_SET;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_ACK_OK)
                    {
                        instance->ops_param.sending_data = (instance->ops_param.reg_addr & 0xFF);
                        instance->ops_param.sending_bit = 8;
                        instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_SDA_SET;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_ACK_OK)
                    {
                        /* sda = x scl = 0 and time is delayed */
                        instance->ops_param.sending_data = instance->device_addr & 0xFF;
                        instance->ops_param.sending_bit = 8;
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_RADDR_SDA_SET;
                    }
                }
                else 
                {
                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_ADDR_ACK_OK)
                    {
                        /* device address finished , send reg address */
                        instance->ops_param.sending_bytes = instance->reg_address_bytes;
                        if(instance->ops_param.sending_bytes == 1)
                        {
                            /* reg address is 8 bit*/
                            instance->ops_param.sending_data = (instance->ops_param.reg_addr & 0xFF);
                            instance->ops_param.sending_bit = 8;
                            instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_SDA_SET;

                        }
                        else if(instance->ops_param.sending_bytes == 2)
                        {
                            /* reg address is 16 bit*/
                            instance->ops_param.sending_data = (instance->ops_param.reg_addr >> 8);
                            instance->ops_param.sending_bit = 8;
                            instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_SDA_SET;
                        }
                        else 
                        {
                            instance->ops_param.ops_state = IO_I2C_OPS_REG_ERROR;
                        }
                    }
                    
                    /* reg address send done*/
                    if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_ACK_OK)
                    {
                        /* reg address finished, send sr or write data */
                        if(instance->state == IO_I2C_READ)
                        {
                            // sr
                            instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                            platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,1);
                            platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,1);

                            instance->ops_param.ops_state = IO_I2C_OPS_SR_SDA0;
                        }
                        else if(instance->state == IO_I2C_WRITE)
                        {
                            // write data
                            instance->ops_param.sending_bytes = instance->ops_param.data_len;
                            instance->ops_param.sending_data = *(instance->ops_param.data);
                            instance->ops_param.sending_bit = 8;
                            instance->ops_param.ops_state = IO_I2C_OPS_DATA_SDA_SET;
                        }
                    }

                    /* read cmd is ok, ready to read data */
                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_ACK_OK)
                    {
                        /* read data */
                        if(instance->state == IO_I2C_READ)
                        {
                            /* sda = x scl = 0 and time is delayed */
                            instance->ops_param.sending_bytes = instance->ops_param.data_len;
                            instance->ops_param.sending_data = 0;
                            instance->ops_param.sending_bit = 8;

                            instance->ops_param.ops_state = IO_I2C_OPS_READ_SCL_SET;
                        }
                        else 
                        {
                            /* should never run here */
                            instance->ops_param.ops_state = IO_I2C_OPS_ERROR;
                        }
                    }
                }
            }
            break;

        case IO_I2C_OPS_READ_SCL_SET:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,1);
                
                instance->ops_param.ops_state = IO_I2C_OPS_READ_SDA_GET;
            }
            break;

        case IO_I2C_OPS_READ_SDA_GET:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                instance->ops_param.sending_data <<= 1;
                sda_val = (platform_ops.io_i2c_pin_read(instance->sda_port, instance->sda_pin) == 0x00) ? 0x00 : 0x01;
                instance->ops_param.sending_data |= sda_val;

                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,0);
                instance->ops_param.sending_bit--;

                if(instance->ops_param.sending_bit)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_READ_SCL_SET;
                }
                else 
                {
                    *(instance->ops_param.data) = instance->ops_param.sending_data;
                    instance->ops_param.data++;                     /* move pointer */
                    instance->ops_param.sending_data = 0x00;
                    instance->ops_param.sending_bytes--;
                    if(instance->ops_param.sending_bytes)
                    {
                        /* send ack */
                        instance->ops_param.ops_state = IO_I2C_OPS_SEND_ACK;

                    }
                    else 
                    {
                        /* send nack */
                        instance->ops_param.ops_state = IO_I2C_OPS_SEND_NACK;
                    }
                }
            }
            break;

        case IO_I2C_OPS_SEND_ACK:
        case IO_I2C_OPS_SEND_NACK:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;

                if(instance->ops_param.ops_state == IO_I2C_OPS_SEND_ACK)
                {
                    platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,0);
                    instance->ops_param.ops_state = IO_I2C_OPS_SEND_ACK_SCL_SET;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_SEND_NACK)
                {
                    platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,1);
                    instance->ops_param.ops_state = IO_I2C_OPS_SEND_NACK_SCL_SET;
                }
                
            }
            break;

        case IO_I2C_OPS_SEND_ACK_SCL_SET:
        case IO_I2C_OPS_SEND_NACK_SCL_SET:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;

                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,1);

                if(instance->ops_param.ops_state == IO_I2C_OPS_SEND_ACK_SCL_SET)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_SEND_ACK_BUS_RELEASE;
                }
                
                if(instance->ops_param.ops_state == IO_I2C_OPS_SEND_NACK_SCL_SET)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_SEND_NACK_BUS_RELEASE;
                }
            }
            break;

        case IO_I2C_OPS_SEND_ACK_BUS_RELEASE:
        case IO_I2C_OPS_SEND_NACK_BUS_RELEASE:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;

                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,0);

                if(instance->ops_param.ops_state == IO_I2C_OPS_SEND_ACK_BUS_RELEASE)
                {
                    platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,1);
                    instance->ops_param.ops_state = IO_I2C_OPS_SEND_ACK_DONE;
                }
                
                if(instance->ops_param.ops_state == IO_I2C_OPS_SEND_NACK_BUS_RELEASE)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_SEND_NACK_DONE;
                }
                
            }
            break;

        case IO_I2C_OPS_SEND_ACK_DONE:
        case IO_I2C_OPS_SEND_NACK_DONE:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                if(instance->ops_param.ops_state == IO_I2C_OPS_SEND_ACK_DONE)
                {
                    instance->ops_param.exceed_tick = 0;
                    // ack done, read next data 
                    instance->ops_param.ops_state = IO_I2C_OPS_READ_SCL_SET;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_SEND_NACK_DONE)
                {
                    instance->ops_param.exceed_tick = instance->ops_param.delay_tick;;
                    instance->ops_param.ops_state = IO_I2C_OPS_SEND_STOP;
                }
            }
            break;

        case IO_I2C_OPS_SEND_STOP:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,0);

                instance->ops_param.ops_state = IO_I2C_OPS_SEND_STOP_SCL_SET;
            }
            break;

        case IO_I2C_OPS_SEND_STOP_SCL_SET:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,1);

                instance->ops_param.ops_state = IO_I2C_OPS_SEND_STOP_SDA_SET;
            }
            break;

        case IO_I2C_OPS_SEND_STOP_SDA_SET:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,1);

                instance->ops_param.ops_state = IO_I2C_OPS_IDLE;
                instance->state = IO_I2C_IDLE;

                // send done call back
                if(instance->callback != NULL)
                {
                    instance->callback(1);
                }
            }
            break;
            break;


        /* Fall through all error */
        case IO_I2C_OPS_DEV_ACK_FAILED:
        case IO_I2C_OPS_REG_ERROR:
        case IO_I2C_OPS_ERROR:
        default:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                instance->ops_param.start_tick = 0;
                instance->ops_param.exceed_tick = 0;
                instance->ops_param.sending_data = 0;
                instance->ops_param.sending_bit = 0;
                instance->state = IO_I2C_ERROR;
                instance->callback(instance->ops_param.ops_state);
            }
            break;
    }

    return 0;
}

int io_i2c_write_reg(struct io_i2c_instance_t *instance, uint16_t reg_addr, uint16_t len, uint8_t *data)
{
    return 0;
}

int io_i2c_read_reg(struct io_i2c_instance_t *instance, uint16_t reg_addr, uint16_t len, uint8_t *data)
{


    if(instance == NULL)
    {
        return -1;
    }
    else 
    {
        if(instance->state != IO_I2C_IDLE)
        {
            return -2;
        }
        else 
        {
            instance->state = IO_I2C_READ;
            instance->ops_param.reg_addr = reg_addr;
            instance->ops_param.data_len = len;
            instance->ops_param.data = data;

            return 0;
        }
    }

    return -100;
}

// static void io_i2c_start(io_i2c_instance_t *instance)
// {

// }

static uint32_t i2c_test_start = 0;
static uint32_t i2c_test_left = 0; // platform_ops.io_i2c_reload_tick * 500;

void io_i2c_test(void)
{
    // static uint32_t val = 0;
    uint8_t touch_id[11];

    // io_i2c_instance_t instance;

    // instance = touch_i2c;

    io_i2c_read_reg(&touch_i2c, 0x8140, 11, touch_id);

#if 0
    if(io_i2c_time_exceed(&i2c_test_start, &i2c_test_left) == 0) 
    {

   
        i2c_test_start = platform_ops.io_i2c_get_tick();
        i2c_test_left = platform_ops.io_i2c_reload_tick * 100;

        val = !val;
        platform_ops.io_i2c_pin_write(instance.sda_port, instance.sda_pin,val);
        platform_ops.io_i2c_pin_write(instance.scl_port, instance.scl_pin,val);
    
        return;
    }
#endif
}

void test_callback(uint8_t event)
{
    static uint8_t test_val = 0;
    if(event == 1)
    {
        test_val++;
    }
}
