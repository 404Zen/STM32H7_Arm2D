/**
 * @file ring_buffer.c
 * @brief Brief description of the source file
 * @author 404zen
 * @date 2026-02-07
 * @version 1.0
 * @note Genrate by AI, NOT TESTED, USE WITH CAUTION!
 */

#include <stdio.h>
#include <string.h>
#include "ring_buffer.h"

rb_error_t rb_init(ring_buffer_t *rb, uint8_t *buffer, uint32_t size) 
{
    if (rb == NULL || buffer == NULL) {
        return RB_ERR_NULL_PTR;
    }
    
    if (size == 0) {
        return RB_ERR_INVALID_SIZE;
    }
    
    rb->buffer = buffer;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    
    return RB_ERR_NONE;
}

uint32_t rb_write(ring_buffer_t *rb, const uint8_t *data, uint32_t len) {
    if (rb == NULL || data == NULL || len == 0) {
        return 0;
    }
    
    // 计算实际可写入的长度
    uint32_t free_space = rb_get_free(rb);
    if (free_space == 0) {
        return 0;
    }
    
    if (len > free_space) {
        len = free_space;  // 截断至可用空间
    }
    
    uint32_t write_len = len;
    
    // 分两阶段写入：从tail到缓冲区末尾，然后从缓冲区开头继续
    uint32_t first_chunk = rb->size - rb->tail;  // 尾部连续空间
    if (first_chunk >= len) {
        // 一次性写入完成
        memcpy(&rb->buffer[rb->tail], data, len);
        rb->tail = (rb->tail + len) % rb->size;
    } else {
        // 需要分两段写入
        memcpy(&rb->buffer[rb->tail], data, first_chunk);
        uint32_t second_chunk = len - first_chunk;
        memcpy(rb->buffer, data + first_chunk, second_chunk);
        rb->tail = second_chunk;
    }
    
    rb->count += write_len;
    return write_len;
}

uint32_t rb_read(ring_buffer_t *rb, uint8_t *data, uint32_t len) 
{
    if (rb == NULL || data == NULL || len == 0) {
        return 0;
    }
    
    // 计算实际可读取的长度
    if (rb->count == 0) {
        return 0;
    }
    
    if (len > rb->count) {
        len = rb->count;  // 截断至可用数据
    }
    
    uint32_t read_len = len;
    
    // 分两阶段读取：从head到缓冲区末尾，然后从缓冲区开头继续
    uint32_t first_chunk = rb->size - rb->head;  // 头部连续数据
    if (first_chunk >= len) {
        // 一次性读取完成
        memcpy(data, &rb->buffer[rb->head], len);
        rb->head = (rb->head + len) % rb->size;
    } else {
        // 需要分两段读取
        memcpy(data, &rb->buffer[rb->head], first_chunk);
        uint32_t second_chunk = len - first_chunk;
        memcpy(data + first_chunk, rb->buffer, second_chunk);
        rb->head = second_chunk;
    }
    
    rb->count -= read_len;
    return read_len;
}

uint32_t rb_read_nocopy(ring_buffer_t *rb, uint8_t **data, uint32_t *rlen)
{
    if (rb == NULL) 
    {
        *data = NULL;
        *rlen = 0;
        return 0;
    }

    uint32_t first_chunk = rb->size - rb->head;
    if (first_chunk > rb->count) 
    {
        first_chunk = rb->count;  // 不能超过有效数据量
    }

    // 返回数据指针和长度
    *data = &(rb->buffer[rb->head]);
    *rlen = first_chunk;

    return first_chunk;
}

//  rb_read_nocopy 配合使用的提交函数（标记数据已读）
uint32_t rb_read_commit(ring_buffer_t *rb, uint32_t len) 
{
    if (rb == NULL || len == 0 || len > rb->count) {
        return 0;
    }

    rb->head = (rb->head + len) % rb->size;
    rb->count -= len;
    
    return len;
}

uint32_t rb_peek(const ring_buffer_t *rb, uint8_t *data, uint32_t len) 
{
    if (rb == NULL || data == NULL || len == 0 || rb->count == 0) {
        return 0;
    }
    
    if (len > rb->count) {
        len = rb->count;
    }
    
    uint32_t temp_head = rb->head;
    uint32_t first_chunk = rb->size - temp_head;
    
    if (first_chunk >= len) {
        memcpy(data, &rb->buffer[temp_head], len);
    } else {
        memcpy(data, &rb->buffer[temp_head], first_chunk);
        memcpy(data + first_chunk, rb->buffer, len - first_chunk);
    }
    
    return len;
}

uint32_t rb_skip(ring_buffer_t *rb, uint32_t len) {
    if (rb == NULL || len == 0) {
        return 0;
    }
    
    if (len > rb->count) {
        len = rb->count;
    }
    
    rb->head = (rb->head + len) % rb->size;
    rb->count -= len;
    
    return len;
}

void rb_clear(ring_buffer_t *rb) {
    if (rb != NULL) {
        rb->head = 0;
        rb->tail = 0;
        rb->count = 0;
    }
}

