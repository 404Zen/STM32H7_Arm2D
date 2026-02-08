/********************************************************************************************************
* @file     key.c
*
* @brief    General Key Driver
*
* @author   404zen
*
* @date     2021-08-19 09:55:00
*
* @attention
*
* None.
*
*******************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "key.h"
#include "time_port.h"
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    uint8_t         priority;                                   //优先级: 不可重复，取值范围 0~(KEY_PAD_NUM-1)
    GPIO_PinState   (*gpio_read)(const GPIO_TypeDef *, uint16_t);     //GPIO读电平函数
    KeyDevice_t     device[KEY_PAD_NUM];
}KeyRegisterTypeDef;

KeyRegisterTypeDef  key_structure;

KeyValue_t          key_value_stack;

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Private user code ---------------------------------------------------------*/
 /**
  * @brief  KeyInit 按键初始化
  * @note   None.
  * @param  None.
  * @retval None.
  */
void KeyInit(void)
{
    KeyDevice_t key_device;

    key_structure.priority = 0;
    key_structure.gpio_read = HAL_GPIO_ReadPin;

    key_device.key_port     = KEY0_GPIO_Port;
    key_device.key_pin      = KEY0_Pin;
    key_device.key_state    = GPIO_PIN_RESET;
    key_device.key_value    = VK_0;
    KeyRegister(&key_device);

//    key_device.key_port     = KEY1_GPIO_Port;
//    key_device.key_pin      = KEY1_Pin;
//    key_device.key_state    = GPIO_PIN_RESET;
//    key_device.key_value    = VK_1;
//    KeyRegister(&key_device);

//    key_device.key_port     = KEY2_GPIO_Port;
//    key_device.key_pin      = KEY2_Pin;
//    key_device.key_state    = GPIO_PIN_RESET;
//    key_device.key_value    = VK_2;
//    KeyRegister(&key_device);

//    key_device.key_port     = KEY3_GPIO_Port;
//    key_device.key_pin      = KEY3_Pin;
//    key_device.key_state    = GPIO_PIN_RESET;
//    key_device.key_value    = VK_3;
//    KeyRegister(&key_device);
}

 /**
  * @brief  vKeySacnTask 按键扫描任务
  * @note   None.
  * @param  None.
  * @retval None.
  */
void vKeySacnTask(void)
{
    static uint8_t keysave;
    static uint8_t key_scan_status = 0;
    static uint32_t key_timer = 0;

    uint8_t i = 0;

    switch (key_scan_status)
    {
    case 0:         /* 按键扫描 */
        for(i = 0; i < key_structure.priority; i++)         //扫描已经注册的按键
        {
            if(key_structure.device[i].key_state == GPIO_PIN_SET)               //高电平有效按键
            {
                if(key_structure.gpio_read(key_structure.device[i].key_port, key_structure.device[i].key_pin) == GPIO_PIN_SET)
                {
                    keysave = i;
                    key_timer = GetSysTime();
                    key_scan_status = 1;
                }
            }
            else
            {
                if(key_structure.gpio_read(key_structure.device[i].key_port, key_structure.device[i].key_pin) == GPIO_PIN_RESET)
                {
                    keysave = i;
                    key_timer = GetSysTime();
                    key_scan_status = 1;
                }
            }
        }
        break;

    case 1:         /* 软件消抖 */
        if(SysTimeExceed(key_timer, KEY_FILTER_TIME) == TIME_IS_ARRIVED)
        {
            if(key_structure.device[keysave].key_state == GPIO_PIN_SET)
            {
                /* 按键未释放，判定为有效按键 */
                if(key_structure.gpio_read(key_structure.device[keysave].key_port, key_structure.device[keysave].key_pin) == GPIO_PIN_SET)
                {

                    #if KEY_PUSH_RELEASE

                    #else
                    KeyPushStack(key_structure.device[keysave].key_value)
                    #endif
                    key_scan_status = 2;
                }
                /* 按键释放了，无效按键 */
                else
                {
                    key_scan_status = 0;    //返回继续扫描
                }
            }
            else
            {
                if(key_structure.gpio_read(key_structure.device[keysave].key_port, key_structure.device[keysave].key_pin) == GPIO_PIN_RESET)
                {
                    #if (KEY_PUSH_RELEASE == 0)
                    KeyPushStack(key_structure.device[keysave].key_value);
                    #endif
                    key_scan_status = 2;
                }
                /* 按键释放了，无效按键 */
                else
                {
                    key_scan_status = 0;    //返回继续扫描
                }
            }
        }
        break;

    case 2:         /* 短按按键等待用户释放按键或到达长按时间阈值 */
        if(key_structure.device[keysave].key_state == GPIO_PIN_SET)
        {
            /* 按键释放了 */
            if(key_structure.gpio_read(key_structure.device[keysave].key_port, key_structure.device[keysave].key_pin) == GPIO_PIN_RESET)
            {
                key_timer = GetSysTime();
                key_scan_status = 3;
                break;
            }
        }
        else
        {
            /* 按键释放了 */
            if(key_structure.gpio_read(key_structure.device[keysave].key_port, key_structure.device[keysave].key_pin) == GPIO_PIN_SET)
            {
                key_timer = GetSysTime();
                key_scan_status = 3;
                break;
            }
        }

        if(SysTimeExceed(key_timer, KEY_LONGPRESS_TIME) == TIME_IS_ARRIVED)
        {
            key_scan_status = 4;
            KeyPushStack(key_structure.device[keysave].key_value | KEY_VALUE_LONGPRESS);
            break;
        }
        break;

    case 3:         /* 确认释放后Push键值 */
        if(SysTimeExceed(key_timer, KEY_FILTER_TIME) == TIME_IS_ARRIVED)
        {
            if(key_structure.device[keysave].key_state == GPIO_PIN_SET)
            {
                /* 按键释放了 */
                if(key_structure.gpio_read(key_structure.device[keysave].key_port, key_structure.device[keysave].key_pin) == GPIO_PIN_RESET)
                {
                    #if KEY_PUSH_RELEASE
                    KeyPushStack(key_structure.device[keysave].key_value);
                    #endif
                }
                key_scan_status = 0;
            }
            else
            {
                /* 按键释放了 */
                if(key_structure.gpio_read(key_structure.device[keysave].key_port, key_structure.device[keysave].key_pin) == GPIO_PIN_SET)
                {
                    #if KEY_PUSH_RELEASE
                    KeyPushStack(key_structure.device[keysave].key_value);
                    #endif
                }
                key_scan_status = 0;
            }
        }
        break;

    case 4:
        if(key_structure.device[keysave].key_state == GPIO_PIN_SET)
        {
            /* 按键释放了 */
            if(key_structure.gpio_read(key_structure.device[keysave].key_port, key_structure.device[keysave].key_pin) == GPIO_PIN_RESET)
            {
                key_timer = GetSysTime();
                key_scan_status = 5;
                break;
            }
        }
        else
        {
            /* 按键释放了 */
            if(key_structure.gpio_read(key_structure.device[keysave].key_port, key_structure.device[keysave].key_pin) == GPIO_PIN_SET)
            {
                key_timer = GetSysTime();
                key_scan_status = 5;
                break;
            }
        }
        break;

    case 5:
        if(SysTimeExceed(key_timer, KEY_FILTER_TIME) == TIME_IS_ARRIVED)
        {
            if(key_structure.device[keysave].key_state == GPIO_PIN_SET)
            {
                /* 按键释放了 */
                if(key_structure.gpio_read(key_structure.device[keysave].key_port, key_structure.device[keysave].key_pin) == GPIO_PIN_RESET)
                {
                    key_scan_status = 0;
                }
                else
                {
                    key_scan_status = 4;
                }
            }
            else
            {
                /* 按键释放了 */
                if(key_structure.gpio_read(key_structure.device[keysave].key_port, key_structure.device[keysave].key_pin) == GPIO_PIN_SET)
                {
                    key_scan_status = 0;
                }
                else
                {
                    key_scan_status = 4;
                }
            }
        }
        break;

    default:
        break;
    }
}

 /**
  * @brief  KeyRegister 按键注册
  * @note   None.
  * @param  *key: 按键结构体指针
  * @retval 1:Success; 0:Fail.
  */
uint8_t KeyRegister(KeyDevice_t *key)
{
    if(key_structure.priority >= KEY_PAD_NUM)
    {
        return 0;
    }

    key_structure.device[key_structure.priority].key_port   = key->key_port;
    key_structure.device[key_structure.priority].key_pin    = key->key_pin;
    key_structure.device[key_structure.priority].key_state  = key->key_state;
    key_structure.device[key_structure.priority].key_value  = key->key_value;

    key_structure.priority++;

    return 1;
}

 /**
  * @brief  KeyDelect 按键删除
  * @note   None.
  * @param  *key: 按键结构体指针
  * @retval 1:Success; 0:Fail,可能不存在此按键 .
  */
uint8_t KeyDelect(KeyDevice_t *key)
{
    uint8_t i = 0;
    uint8_t found_key = KEY_PAD_NUM + 1;

    /* 键值是唯一的，根据按键键值来查找按键 */
    for (i = 0; i <= key_structure.priority; i++)
    {
        if(key->key_value == key_structure.device[i].key_value)
        {
            found_key = i;
            break;
        }
    }

    if(found_key >= KEY_PAD_NUM)
    {
        return 0;
    }
    else
    {
        /* 移动原来的按键往前格 */
        for (i = found_key+1; i < key_structure.priority; i++)
        {
            key_structure.device[i-1].key_port  = key_structure.device[i].key_port;
            key_structure.device[i-1].key_pin   = key_structure.device[i].key_pin;
            key_structure.device[i-1].key_state = key_structure.device[i].key_state;
            key_structure.device[i-1].key_value = key_structure.device[i].key_value;
        }
        key_structure.device[key_structure.priority].key_value = 0;
        key_structure.priority--;

        return 1;
    }
}

 /**
  * @brief  KeyPushStack 按键入栈
  * @note   None.
  * @param  key_value: 要入栈的按键键值
  * @retval None.
  */
void KeyPushStack(uint8_t key_value)
{
#if 0
    /* 按键缓存没满的情况下，存到缓存尾部 */
    if(key_value_stack.counter < KEY_VALUE_BUF_MAX)
    {
        key_value_stack.key_value[key_value_stack.counter++] = key_value;
    }
    /* 按键缓存满，替代最后一个 */
    else
    {
        key_value_stack.key_value[KEY_VALUE_BUF_MAX-1] = key_value;
    }
#else
    if(key_value_stack.buf_used < KEY_VALUE_BUF_MAX-1)
    {
        key_value_stack.key_value[key_value_stack.in_counter++] = key_value;
        key_value_stack.buf_used++;
        if (key_value_stack.in_counter >= KEY_VALUE_BUF_MAX)
        {
            key_value_stack.in_counter = 0;
        }
    }
    else
    {
        key_value_stack.key_value[key_value_stack.in_counter] = key_value;
    }


#endif
}

 /**
  * @brief  KeyPopStack 按键出栈
  * @note   None.
  * @param  None.
  * @retval 按键键值.
  */
uint8_t KeyPopStack(void)
{
    uint8_t kv = VK_NOKEYPRESS;

    if(key_value_stack.buf_used)
    {
        kv = key_value_stack.key_value[key_value_stack.out_counter++];
        key_value_stack.buf_used--;

        if(key_value_stack.out_counter >= KEY_VALUE_BUF_MAX)
        {
            key_value_stack.out_counter = 0;
        }

        return kv;
    }
    else
    {
        return VK_NOKEYPRESS;
    }
}

 /**
  * @brief  KeyFunctionTest 按键测试函数
  * @note   None.
  * @param  None.
  * @retval 按键键值.
  */
void KeyFunctionTest(void)
{
    uint8_t kv = 0;

    // vKeySacnTask();

    kv = KeyPopStack();

    if(kv)
    {
        KEY_DEBUG("Key Press, key value is 0x%X\r\n", kv);
        kv = 0;
    }
}









/*********************************END OF FILE**********************************/
