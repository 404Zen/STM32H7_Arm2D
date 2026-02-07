/**
 * @file ring_buffer.h
 * @brief Brief description of the header file
 * @author 404zen
 * @date 2026-02-07
 * @version 1.0
 */

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// 错误码定义
typedef enum {
    RB_ERR_NONE = 0,
    RB_ERR_NULL_PTR,
    RB_ERR_INVALID_SIZE,
    RB_ERR_BUFFER_FULL,
    RB_ERR_BUFFER_EMPTY
} rb_error_t;

// 环形缓冲区结构体
typedef struct {
    uint8_t *buffer;      // 指向静态内存缓冲区
    uint32_t size;        // 缓冲区总大小
    uint32_t head;        // 读指针（消费者位置）
    uint32_t tail;        // 写指针（生产者位置）
    uint32_t count;       // 当前数据量
} ring_buffer_t;

/**
 * @brief 初始化环形缓冲区
 * @param rb 环形缓冲区实例指针
 * @param buffer 外部静态内存缓冲区指针
 * @param size 缓冲区大小
 * @return 错误码
 */
rb_error_t rb_init(ring_buffer_t *rb, uint8_t *buffer, uint32_t size);

/**
 * @brief 向缓冲区写入数据
 * @param rb 环形缓冲区实例指针
 * @param data 要写入的数据指针
 * @param len 要写入的数据长度
 * @return 实际写入的数据长度
 */
uint32_t rb_write(ring_buffer_t *rb, const uint8_t *data, uint32_t len);

/**
 * @brief 从缓冲区读取数据
 * @param rb 环形缓冲区实例指针
 * @param data 数据读取缓冲区指针
 * @param len 期望读取的数据长度
 * @return 实际读取的数据长度
 */
uint32_t rb_read(ring_buffer_t *rb, uint8_t *data, uint32_t len);

uint32_t rb_read_nocopy(ring_buffer_t *rb, uint8_t **data, uint32_t *rlen);
uint32_t rb_read_commit(ring_buffer_t *rb, uint32_t len);
/**
 * @brief 查看缓冲区数据（不移动读指针）
 * @param rb 环形缓冲区实例指针
 * @param data 数据读取缓冲区指针
 * @param len 期望查看的数据长度
 * @return 实际查看的数据长度
 */
uint32_t rb_peek(const ring_buffer_t *rb, uint8_t *data, uint32_t len);

/**
 * @brief 丢弃指定长度的数据
 * @param rb 环形缓冲区实例指针
 * @param len 要丢弃的数据长度
 * @return 实际丢弃的数据长度
 */
uint32_t rb_skip(ring_buffer_t *rb, uint32_t len);

/**
 * @brief 获取缓冲区中当前数据量
 * @param rb 环形缓冲区实例指针
 * @return 当前数据量
 */
static inline uint32_t rb_get_count(const ring_buffer_t *rb) {
    return (rb != NULL) ? rb->count : 0;
}

/**
 * @brief 获取缓冲区剩余空间
 * @param rb 环形缓冲区实例指针
 * @return 剩余空间大小
 */
static inline uint32_t rb_get_free(const ring_buffer_t *rb) {
    return (rb != NULL) ? (rb->size - rb->count) : 0;
}

/**
 * @brief 检查缓冲区是否为空
 * @param rb 环形缓冲区实例指针
 * @return true-空, false-非空
 */
static inline bool rb_is_empty(const ring_buffer_t *rb) {
    return (rb == NULL) || (rb->count == 0);
}

/**
 * @brief 检查缓冲区是否已满
 * @param rb 环形缓冲区实例指针
 * @return true-满, false-未满
 */
static inline bool rb_is_full(const ring_buffer_t *rb) {
    return (rb != NULL) && (rb->count >= rb->size);
}

/**
 * @brief 清空缓冲区
 * @param rb 环形缓冲区实例指针
 */
void rb_clear(ring_buffer_t *rb);

#ifdef __cplusplus
}
#endif

#endif /* __RING_BUFFER_H__ */
