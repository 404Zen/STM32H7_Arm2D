/**
 * @file io_i2c.h
 * @brief Brief description of the header file
 * @author 404zen
 * @date 2026-02-10
 * @version 1.0
 */

#ifndef __IO_I2C_H__
#define __IO_I2C_H__

#include <stdint.h>
#include <sys/_intsup.h>
#include "main.h"
#include "stm32h7xx_hal_gpio.h"
#include "async_uart.h"

#define IO_I2C_DEBUG_ENABLE               0

#if IO_I2C_DEBUG_ENABLE
    #define i2c_printf(...)              debug_printf(__VA_ARGS__)
#else
    #define i2c_printf(...)              (void)0
#endif

typedef void (*i2c_pin_write_t)(void *port, uint16_t pin, uint32_t value);
typedef uint32_t (*i2c_pin_read_t)(void *port, uint16_t pin);
typedef uint32_t (*i2c_get_tick_t)(void);
typedef void (*io_i2c_callback_t)(uint8_t event);

static inline void io_i2c_write(void *port, uint16_t pin, uint32_t value)
{
    HAL_GPIO_WritePin(port, pin, value);
}

static inline uint32_t io_i2c_read_adapter(void *port, uint16_t pin)
{
    return (uint32_t)HAL_GPIO_ReadPin(port, pin);
}

static inline uint32_t io_i2c_get_tick(void)
{
    return user_systick_value_get();
}

typedef enum : uint8_t
{
    IO_I2C_IDLE = 0,
    
    /* Read */
    IO_I2C_READ,

    /* Write */
    IO_I2C_WRITE,

    IO_I2C_ERROR,
}io_i2c_state;

typedef enum : uint8_t
{
    /* Start */
    IO_I2C_OPS_IDLE = 0,                // SDA = 1, SCL = 1
    IO_I2C_OPS_START_SDA0,              // SDA = 0, SCL = 1
    IO_I2C_OPS_START_SCL0,              // SDA = 0, SCL = 0

    /* Sr */
    IO_I2C_OPS_SR_IDLE,
    IO_I2C_OPS_SR_SDA0,
    IO_I2C_OPS_SR_SCL0,

    /* device address */
    IO_I2C_OPS_DEV_ADDR_SDA_SET,
    IO_I2C_OPS_DEV_ADDR_SCL_SET,
    IO_I2C_OPS_DEV_ADDR_SCL_RESET,
    IO_I2C_OPS_DEV_ADDR_ACK,
    IO_I2C_OPS_DEV_ADDR_ACK_READ,
    IO_I2C_OPS_DEV_ADDR_ACK_OK,

    /* device address read */
    IO_I2C_OPS_DEV_RADDR_SDA_SET,
    IO_I2C_OPS_DEV_RADDR_SCL_SET,
    IO_I2C_OPS_DEV_RADDR_SCL_RESET,
    IO_I2C_OPS_DEV_RADDR_ACK,
    IO_I2C_OPS_DEV_RADDR_ACK_READ,
    IO_I2C_OPS_DEV_RADDR_ACK_OK,
    
    /* reg address */
    IO_I2C_OPS_REG_ADDR_SDA_SET,
    IO_I2C_OPS_REG_ADDR_SCL_SET,
    IO_I2C_OPS_REG_ADDR_SCL_RESET,
    IO_I2C_OPS_REG_ADDR_ACK,
    IO_I2C_OPS_REG_ADDR_ACK_READ,
    IO_I2C_OPS_REG_ADDR_ACK_OK,


    /* read data */
    IO_I2C_OPS_READ_SCL_SET,
    IO_I2C_OPS_READ_SDA_GET,
    IO_I2C_OPS_READ_SDA_RESET,

    IO_I2C_OPS_DATA_SDA_SET,
    IO_I2C_OPS_DATA_SCL_SET,
    IO_I2C_OPS_DATA_SCL_RESET,

    IO_I2C_OPS_DATA_ACK,
    IO_I2C_OPS_DATA_ACK_READ,
    IO_I2C_OPS_DATA_ACK_OK,

    IO_I2C_OPS_SEND_ACK,
    IO_I2C_OPS_SEND_ACK_SCL_SET,
    IO_I2C_OPS_SEND_ACK_BUS_RELEASE,
    IO_I2C_OPS_SEND_ACK_DONE,

    IO_I2C_OPS_SEND_NACK,
    IO_I2C_OPS_SEND_NACK_SCL_SET,
    IO_I2C_OPS_SEND_NACK_BUS_RELEASE,
    IO_I2C_OPS_SEND_NACK_DONE,

    IO_I2C_OPS_SEND_STOP,
    IO_I2C_OPS_SEND_STOP_SCL_SET,
    IO_I2C_OPS_SEND_STOP_SDA_SET,

    IO_I2C_OPS_DEV_ACK_FAILED,
    IO_I2C_OPS_REG_ERROR,
    IO_I2C_OPS_ERROR,
}io_i2c_ops_state_t;

typedef struct
{
    uint16_t    reg_addr;
    uint16_t    data_len;
    uint8_t     *data;



    uint32_t    start_tick;
    uint32_t    exceed_tick;
    uint32_t    delay_tick;

    /* parameters using in loop task */
    uint8_t     sending_data;
    uint8_t     sending_bit;

    uint16_t    sending_bytes;
    
    io_i2c_ops_state_t ops_state;
}io_i2c_ops_param_t;


typedef struct io_i2c_instance_t
{
    
    void            *scl_port;
    uint16_t        scl_pin;
    void            *sda_port;
    uint16_t        sda_pin;
    uint8_t         instance_id;            // not used.       
    uint16_t        device_addr;
    uint8_t         device_addr_mode;       // 0 - 7 bit, 1 - 10 bit 
    uint8_t         reg_address_bytes;      // 1 - 8 bit, 1 - 16 bit    // more than 16bit reg address is not support now
    uint16_t        speed;                  // speed in KHz, must >= 1KHz
    io_i2c_callback_t callback;

    // automatic calculate value
    io_i2c_state        state;
    io_i2c_ops_param_t  ops_param;

    struct io_i2c_instance_t *next;         // next instance
}io_i2c_instance_t;


typedef struct
{
    i2c_pin_write_t io_i2c_pin_write;
    i2c_pin_read_t  io_i2c_pin_read;
    i2c_get_tick_t  io_i2c_get_tick;
    uint32_t        io_i2c_reload_tick;
}io_i2c_platform_t;



void io_i2c_init(void);

void io_i2c_loop_task(void);
int io_i2c_instance_register(struct io_i2c_instance_t *instance);
int io_i2c_write_reg(struct io_i2c_instance_t *instance, uint16_t reg_addr, uint16_t len, uint8_t *data);
int io_i2c_read_reg(struct io_i2c_instance_t *instance, uint16_t reg_addr, uint16_t len, uint8_t *data);


#endif /* __IO_I2C_H__ */
