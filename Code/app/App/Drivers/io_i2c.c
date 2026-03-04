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
#include "main.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"


static io_i2c_platform_t platform_ops;

struct io_i2c_instance_t touch_i2c;
static struct io_i2c_instance_t *loop_list = NULL;


static int io_i2c_time_exceed(uint32_t *start_time, uint32_t *time_out);
static int io_i2c_state_machine(struct io_i2c_instance_t *instance);

void io_i2c_init(void)
{
    /* Platform init */
    platform_ops.io_i2c_pin_write = io_i2c_write;
    platform_ops.io_i2c_pin_read = io_i2c_read_adapter;
    platform_ops.io_i2c_get_tick = io_i2c_get_tick;
    platform_ops.io_i2c_reload_tick = user_systick_reload_get() + 1;            // 280000 = 1ms
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
    /* 100 KHz = 10 us = 2800 ticks, test in 11.5uS */ 
    loop_list->ops_param.delay_tick = sys_core_clk/1000/loop_list->speed/2;


    loop_list->next = NULL;

    platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin, 1);
    platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin, 1);

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
            case IO_I2C_WRITE:
                io_i2c_state_machine(list);
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

static int io_i2c_state_machine(struct io_i2c_instance_t *instance)
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
            i2c_printf("IDLE trans to START\r\n");
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
                    i2c_printf("SR sending\r\n");
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
                    i2c_printf("Start signal sent, send device address 0x%02X, dev address mode %d\r\n", instance->device_addr, instance->device_addr_mode);
                    instance->ops_param.sending_bit = 8;
                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_SDA_SET;
                }

                if(instance->ops_param.ops_state  == IO_I2C_OPS_SR_SCL0)
                {
                    /* Read data*/
                    /* 7 bits address */ 
                    if(instance->device_addr_mode == 0)
                    {
                        i2c_printf("Setting 8bit read device address\r\n");
                        instance->ops_param.sending_data = ((instance->device_addr) | 0x01);
                        instance->ops_param.sending_bytes = 1;
                    }
                    else 
                    {
                        i2c_printf("Setting 16bit read device address\r\n");
                        instance->ops_param.sending_data = (((instance->device_addr) >> 8) | 0x01);
                        instance->ops_param.sending_bytes = 2;
                    }
                    instance->ops_param.sending_bit = 8;
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
                if((instance->ops_param.sending_bit)--)
                {
                    sda_val =((instance->ops_param.sending_data & (0x01 << instance->ops_param.sending_bit)) == 0x00) ? 0 : 1;
                    i2c_printf("%d.", sda_val);
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
                        i2c_printf("Device address is set, wait device ack!\r\n");
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_ACK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_SDA_SET)
                    {
                        i2c_printf("Reg address is set, wait device ack!\r\n");
                        instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_ACK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DATA_SDA_SET)
                    {
                        instance->ops_param.ops_state = IO_I2C_OPS_DATA_ACK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_SDA_SET)
                    {
                        i2c_printf("Read device address is set, wait device ack!\r\n");
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

                if(instance->ops_param.ops_state == IO_I2C_OPS_DATA_ACK)
                {
                    instance->ops_param.ops_state = IO_I2C_OPS_DATA_ACK_READ;
                }
            }
            break;

        /* sda = ack, scl = 1->0, read ack and the pull down scl */
        case IO_I2C_OPS_DEV_ADDR_ACK_READ:
        case IO_I2C_OPS_REG_ADDR_ACK_READ:
        case IO_I2C_OPS_DEV_RADDR_ACK_READ:
        case IO_I2C_OPS_DATA_ACK_READ:
            if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                /* no need get new start tick ,it is updated in io_i2c_time_exceed() */ 
                instance->ops_param.exceed_tick = instance->ops_param.delay_tick;

                if(platform_ops.io_i2c_pin_read(instance->sda_port, instance->sda_pin) != 0)
                {
                    /* device no ack */
                    i2c_printf("ack failed, last state is %d\r\n", instance->ops_param.ops_state);
                    instance->ops_param.ops_state = IO_I2C_OPS_DEV_ACK_FAILED;
                }
                else 
                {
                    /* device ack */
                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_ADDR_ACK_READ)
                    {   
                        i2c_printf("dev address ack ok\r\n");
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_ADDR_ACK_OK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_ACK_READ)
                    {   
                        i2c_printf("reg address ack ok\r\n");
                        instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_ACK_OK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_ACK_READ)
                    {   
                        i2c_printf("read device address ack ok\r\n");
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_RADDR_ACK_OK;
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DATA_ACK_READ)
                    {   
                        i2c_printf("write data ack ok\r\n");
                        instance->ops_param.ops_state = IO_I2C_OPS_DATA_ACK_OK;
                    }
                }

                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,0);
            }
            break;

        /* SDA = 1， SCL = 0 */
        case IO_I2C_OPS_DEV_ADDR_ACK_OK:
        case IO_I2C_OPS_REG_ADDR_ACK_OK:
        case IO_I2C_OPS_DEV_RADDR_ACK_OK:
        case IO_I2C_OPS_DATA_ACK_OK:
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
                        i2c_printf("device address ack ok, setting next 8bit address\r\n");
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_REG_ADDR_ACK_OK)
                    {
                        instance->ops_param.sending_data = (instance->ops_param.reg_addr & 0xFF);
                        instance->ops_param.sending_bit = 8;
                        instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_SDA_SET;
                        i2c_printf("reg addr ack ok, sending next 8bit address\r\n");
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_RADDR_ACK_OK)
                    {
                        /* sda = x scl = 0 and time is delayed */
                        instance->ops_param.sending_data = instance->device_addr & 0xFF;
                        instance->ops_param.sending_bit = 8;
                        instance->ops_param.ops_state = IO_I2C_OPS_DEV_RADDR_SDA_SET;
                        i2c_printf("first 8bit of 16bit read device address set ok\r\n");
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DATA_ACK_OK)
                    {
                        instance->ops_param.data++;
                        instance->ops_param.sending_data = *(instance->ops_param.data);
                        instance->ops_param.sending_bit = 8;
                        instance->ops_param.ops_state = IO_I2C_OPS_DATA_SDA_SET;
                        i2c_printf("write data ack ok, sending next data byte\r\n");
                    }
                }
                else 
                {
                    if(instance->ops_param.ops_state == IO_I2C_OPS_DEV_ADDR_ACK_OK)
                    {
                        i2c_printf("device address set done!\r\n");
                        /* device address finished , send reg address */
                        instance->ops_param.sending_bytes = instance->reg_address_bytes;
                        if(instance->ops_param.sending_bytes == 1)
                        {
                            /* reg address is 8 bit*/
                            instance->ops_param.sending_data = (instance->ops_param.reg_addr & 0xFF);
                            instance->ops_param.sending_bit = 8;
                            instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_SDA_SET;

                            i2c_printf("set 8bit reg address 0x%02X\r\n", instance->ops_param.sending_data);
                        }
                        else if(instance->ops_param.sending_bytes == 2)
                        {
                            /* reg address is 16 bit*/
                            instance->ops_param.sending_data = (instance->ops_param.reg_addr >> 8);
                            instance->ops_param.sending_bit = 8;
                            instance->ops_param.ops_state = IO_I2C_OPS_REG_ADDR_SDA_SET;

                            i2c_printf("set 16bit reg address 0x%04X 1/2 \r\n", instance->ops_param.reg_addr);
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

                            i2c_printf("Reg address set ok, READ data, next is SR\r\n");
                        }
                        else if(instance->state == IO_I2C_WRITE)
                        {
                            // write data
                            instance->ops_param.sending_bytes = instance->ops_param.data_len;
                            instance->ops_param.sending_data = *(instance->ops_param.data);
                            instance->ops_param.sending_bit = 8;
                            instance->ops_param.ops_state = IO_I2C_OPS_DATA_SDA_SET;
                            i2c_printf("Reg address set ok, WRITE data, next is DATA\r\n");
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

                            i2c_printf("Ready to read data %d * %d bits.\r\n", instance->ops_param.sending_bytes, instance->ops_param.sending_bit);
                        }
                        else 
                        {
                            /* should never run here */
                            instance->ops_param.ops_state = IO_I2C_OPS_ERROR;
                        }
                    }

                    if(instance->ops_param.ops_state == IO_I2C_OPS_DATA_ACK_OK)
                    {
                        /* write data finished, send stop signal */
                        instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                        platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,0);
                        platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,1);

                        instance->ops_param.ops_state = IO_I2C_OPS_SEND_STOP;

                        i2c_printf("Write data finished, next is STOP\r\n");
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

                i2c_printf("%d.", sda_val);

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
                        i2c_printf("read not finish, send ack\r\n");
                        instance->ops_param.ops_state = IO_I2C_OPS_SEND_ACK;
                    }
                    else 
                    {
                        /* send nack */
                        i2c_printf("read finished, send nack\r\n");
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
                    instance->ops_param.sending_bit = 8;
                    // ack done, read next data 
                    instance->ops_param.ops_state = IO_I2C_OPS_READ_SCL_SET;
                }

                if(instance->ops_param.ops_state == IO_I2C_OPS_SEND_NACK_DONE)
                {
                    instance->ops_param.exceed_tick = instance->ops_param.delay_tick;
                    i2c_printf("Send STOP\r\n");
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

                // transmit done call back
                if(instance->callback != NULL)
                {
                    instance->callback(0);
                }
            }
            break;
            break;


        /* Fall through all error */
        case IO_I2C_OPS_DEV_ACK_FAILED:
        case IO_I2C_OPS_REG_ERROR:
        case IO_I2C_OPS_ERROR:
        default:
            // if(io_i2c_time_exceed(&(instance->ops_param.start_tick), &(instance->ops_param.exceed_tick)) == 0)
            {
                // instance->ops_param.start_tick = 0;
                // instance->ops_param.exceed_tick = 0;
                // instance->ops_param.sending_data = 0;
                // instance->ops_param.sending_bit = 0;
                platform_ops.io_i2c_pin_write(instance->sda_port, instance->sda_pin,1);
                platform_ops.io_i2c_pin_write(instance->scl_port, instance->scl_pin,1);
                i2c_printf("ERROR, last state is %d\r\n", instance->ops_param.ops_state);
                instance->state = IO_I2C_ERROR;
                instance->callback(instance->ops_param.ops_state);
            }
            break;
    }

    return 0;
}

int io_i2c_write_reg(struct io_i2c_instance_t *instance, uint16_t reg_addr, uint16_t len, uint8_t *data)
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
            instance->state = IO_I2C_WRITE;
            instance->ops_param.reg_addr = reg_addr;
            instance->ops_param.data_len = len;
            instance->ops_param.data = data;

            return 0;
        }
    }
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

