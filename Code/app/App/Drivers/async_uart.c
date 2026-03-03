/**
 * @file async_uart.c
 * @brief Brief description of the source file
 * @author 404zen
 * @date 2026-02-07
 * @version 1.0
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "async_uart.h"
#include "ring_buffer.h"
#include "stm32h7xx_hal_def.h"

/* Port to your platform */
extern UART_HandleTypeDef huart1;

/* Data buffer */
__attribute__((section(".sram_noncache_bss"))) uint8_t uart1_tx_buf[2048];


/* Declear instance */
async_uart_instance_t uart1;


uint16_t async_uart_vprintf(async_uart_instance_t *instance, const char *__format, va_list args);

static inline int32_t platform_uart_async_send(void *hw_instance, uint8_t *data, uint32_t len)
{
    int32_t ret = 0;
    ret = (int32_t)HAL_UART_Transmit_DMA((UART_HandleTypeDef *)hw_instance, data, len);

    if(ret == (int32_t)HAL_OK)
    {
        return 0;
    }
    else 
    {
        return -1;
    }
}

__attribute__((section(".fast_code"))) void async_uart_callback(void *hw_instance, async_uart_event event)
{
    if(((UART_HandleTypeDef *)hw_instance)->Instance == USART1)
    {
        if (event == AU_EVENT_TRASNMIT_COMPLETE)
        {
            if(rb_get_count(&(uart1.tx_buffer)) > 0)
            {
                uint8_t *send_data_ptr = NULL;
                uint32_t send_len = 0;
                rb_read_nocopy(&(uart1.tx_buffer), &send_data_ptr, &send_len);
                platform_uart_async_send(uart1.hw_instance, send_data_ptr, send_len);
                rb_read_commit(&(uart1.tx_buffer), send_len);
            }
            else
            {
                uart1.tx_status = ASYNC_UART_IDLE;
            }
        }
        else if (event == AU_EVENT_TRASNMIT_ERROR)
        {
            uart1.tx_status = ASYNC_UART_ERROR;
        }
    }
}




void async_uart_init(void)
{
    /* uart1 initialize */
    rb_init(&(uart1.tx_buffer), uart1_tx_buf, sizeof(uart1_tx_buf));
    uart1.tx_status = ASYNC_UART_IDLE;
    // uart1.rx_buffer = NULL;  
    uart1.rx_status = ASYNC_UART_IDLE;
    uart1.hw_instance = (void *)&huart1;
}



int32_t async_uart_send(async_uart_instance_t *instance, uint8_t *data, uint32_t len)
{
    uint8_t *send_data_ptr = NULL;
    uint32_t send_len = 0;
    int32_t written_len = 0;

    if(instance == NULL || data == NULL || len == 0)
    {
        return -1;
    }

    if(len > rb_get_free(&(instance->tx_buffer)))
    {
        return -2;  // Not enough space in buffer
    }

    // Write all data to buffer
    written_len = (int32_t)rb_write(&(instance->tx_buffer), data, len);

    if(instance->tx_status == ASYNC_UART_IDLE)
    {
        // if buffer is exceed int32_t max, return value is error, but data is still sent correctly, so do something fix it?
        if(written_len)
        {
            instance->tx_status = ASYNC_UART_BUSY;
            rb_read_nocopy(&(instance->tx_buffer), &send_data_ptr, &send_len);
            
            platform_uart_async_send(instance->hw_instance, send_data_ptr, send_len);

            rb_read_commit(&(instance->tx_buffer), send_len);
        }
    }

    return written_len;            
}



uint16_t async_uart_vprintf(async_uart_instance_t *instance, const char *__format, va_list args)
{
    char strbuf[256] = {0};
    uint16_t formatted_len = 0;
    uint16_t written_len = 0;

    // 格式化字符串
    formatted_len = vsnprintf((char *)strbuf, sizeof(strbuf), __format, args);

    // 检查格式化是否成功
    if (formatted_len >= sizeof(strbuf))
    {
        formatted_len = sizeof(strbuf) - 1;
        strbuf[formatted_len] = '\0';
    }

    if(formatted_len)
    {
        written_len = async_uart_send(instance, (uint8_t *)strbuf, formatted_len);
    }

    return written_len;
}

uint16_t async_usart_printf(async_uart_instance_t *instance, const char *__format, ...)
{
#if 0
    char strbuf[256] = {0};
    uint16_t len = 0;
    va_list list;

    va_start(list, __format);
    len = vsnprintf(strbuf, sizeof(strbuf), __format, list);
    va_end(list);

    if(len)
    {
        async_uart_send(instance, (uint8_t *)strbuf, len);
    }
#endif
    va_list args;
    uint16_t result;
    
    va_start(args, __format);
    result = async_uart_vprintf(instance, __format, args);
    va_end(args);
    
    return result;
}

/**
 * @brief  Send data via usart1
 * @param[in]  param1 Description of the input parameter
 * @param[out] param2 Description of the output parameter
 * @retval HAL status
 * @note   Additional notes, e.g., This function must be called after initialization.
 * @warning Warnings, e.g., This function is not thread-safe.
 */

void debug_printf(const char *__format, ...)
{
    va_list args;
    va_start(args, __format);
    async_uart_vprintf(&uart1, __format, args);
    va_end(args);
}


