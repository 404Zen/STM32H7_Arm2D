/********************************************************************************************************
* @file     key.h
*
* @brief    General Key Driver header file
*
* @author   404zen
*
* @date     2021-08-19 09:56:49
*
* @attention
*
* None.
*
*******************************************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __KEY_H__
#define __KEY_H__

/* Includes ------------------------------------------------------------------*/
// #include "stm32h7xx_hal_gpio.h"
#include "main.h"
#include <stdio.h>
#include "async_uart.h"
#include "stm32h7xx_hal_gpio.h"

/* Private includes ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

#define KEY_PAD_NUM                             1           //按键数量
#define KEY_VALUE_BUF_MAX                       3           //按键buf缓存数量

#define KEY_PUSH_RELEASE                        1           //短按按键释放之后才Push键值


#define KEY_FILTER_TIME                         30           //按键消抖时间
#define KEY_LONGPRESS_TIME                      1000        //按键长按时间      长按按键到达长按时间马上Push键值

//Personal Keyvalue define
#define KEY_VALUE_SET                           VK_RETURN   //Only one key in this project
#define KEY_VALUE_LONGPRESS                     0x80        //长按按键


#if 1
extern async_uart_instance_t uart1;
#define KEY_DEBUG(...)													async_usart_printf(&uart1, __VA_ARGS__)
#else
#define KEY_DEBUG(...)													(void*)0
#endif

#if 0
#define GetSysTime(...)							(void*)0
#define SysTimeExceed(...)						(void*)0
#define TIME_IS_ARRIVED							0
#endif


//标准按键键值表
#define VK_NOKEYPRESS                           0x00        //没有按键按下 ，空键值
#define VK_LBUTTON                              0x01        //鼠标左键
#define VK_RBUTTON                              0x02        //鼠标右键
#define VK_CANCEL                               0x03        //Ctrl+Break
#define VK_M_BUTTON                             0x04        //鼠标中键
#define VK_BACK                                 0x05        //Backspace
#define VK_TAB                                  0x06        //Tab
#define VK_CLEAR                                0x0C        //Clear
#define VK_RETURN                               0x0D        //Enter
#define VK_SHIFT                                0x10        //Shitf
#define VK_CONTROL                              0x11        //Control
#define VK_MENU                                 0x12        //Alt
#define VK_PAUSE                                0x13        //Pause
#define VK_CAPITAL                              0x14        //Caps Lock
#define VK_ESCAPE                               0x1B        //Esc
#define VK_SPACE                                0x20        //Space
#define VK_LEFT                                 0x25        //Left arrow
#define VK_UP                                   0x26        //Up arrow
#define VK_RIGHT                                0x27        //Right arrow
#define VK_DOWN                                 0x28        //Down arrow

#define VK_0                                    0x30        //0
#define VK_1                                    0x31        //1
#define VK_2                                    0x32        //2
#define VK_3                                    0x33        //3

/* Exported types ------------------------------------------------------------*/
/* 按键注册结构体 */
typedef struct
{
    GPIO_TypeDef    *key_port;                              //端口
    uint32_t        key_pin;                                //引脚
    GPIO_PinState   key_state;                              //按键有效电平
    uint8_t         key_value;                              //按键键值
}KeyDevice_t;


/* 键值缓存数据结构体 */
typedef struct
{
    uint8_t         in_counter;                             //计数器
    uint8_t         out_counter;
    uint8_t         buf_used;                               //已使用的缓存
    uint8_t         key_value[KEY_VALUE_BUF_MAX];           //按键键值缓存
}KeyValue_t;


/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void KeyInit(void);                                         //按键初始化
void vKeySacnTask(void);                                    //按键扫描任务
uint8_t KeyRegister(KeyDevice_t *key);                      //按键注册
uint8_t KeyDelect(KeyDevice_t *key);                        //按键删除
void KeyPushStack(uint8_t key_value);                       //按键入栈
uint8_t KeyPopStack(void);                                  //按键出栈

void KeyFunctionTest(void);                                 //按键测试函数






#endif /* __KEY_H__ */
/*********************************END OF FILE**********************************/
