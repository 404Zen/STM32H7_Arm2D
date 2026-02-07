/**
 * @file async_uart.h
 * @brief Brief description of the header file
 * @author 404zen
 * @date 2026-02-07
 * @version 1.0
 */

#ifndef __ASYNC_UART_H__
#define __ASYNC_UART_H__

#include "ring_buffer.h"
#include <stdint.h>



typedef enum
{
    AU_EVENT_TRASNMIT_COMPLETE = 0,
    AU_EVENT_TRASNMIT_ERROR,
}async_uart_event;

typedef enum
{
    ASYNC_UART_IDLE = 0,
    ASYNC_UART_BUSY,
    ASYNC_UART_OVERFLOW,
    ASYNC_UART_ERROR,
}async_uart_status;

typedef struct 
{
    void                *hw_instance;
    async_uart_status   tx_status;
    ring_buffer_t       tx_buffer;
    async_uart_status   rx_status;
    ring_buffer_t       rx_buffer;
}async_uart_instance_t;

void async_uart_init(void);
void async_uart_callback(void *hw_instance, async_uart_event event);
int32_t async_uart_send(async_uart_instance_t *instance, uint8_t *data, uint32_t len);
void async_usart_printf(async_uart_instance_t *instance, const char *__format, ...);


#endif /* __ASYNC_UART_H__ */
