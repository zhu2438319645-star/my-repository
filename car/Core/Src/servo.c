#include "servo.h"
#include <math.h>
static uint32_t _angle_to_compare(Servo_HandleTypeDef *servo, uint16_t angle);
static void _write_compare(Servo_HandleTypeDef *servo, uint32_t compare);

Servo_HandleTypeDef servo_handles[SERVO_NUM];

// 舵机初始化函数，对结构体进行初始化，打开pwm通道等
void Servo_Init(Servo_ID id, TIM_HandleTypeDef *htim, uint32_t channel,
                uint16_t min_angle, uint16_t max_angle)
{
    servo_handles[id].id = id;
    servo_handles[id].htim = htim;
    servo_handles[id].channel = channel;
    servo_handles[id].min_angle = min_angle;
    servo_handles[id].max_angle = max_angle;

    // 启动pwm通道
    HAL_TIM_PWM_Start(htim, channel);

    // 设置初始值，转到中间位置
    uint16_t mid_angle = (min_angle + max_angle) / 2;
    Servo_SetAngle(id, mid_angle);
    HAL_Delay(100);
}

// 舵机旋转角度处理一下，确保在可以转动的角度里
void Servo_SetAngle(Servo_ID id, uint16_t angle)
{
    if (angle < servo_handles[id].min_angle)
    {
        angle = servo_handles[id].min_angle;
    }
    else if (angle > servo_handles[id].max_angle)
    {
        angle = servo_handles[id].max_angle;
    }

    Servo_SetAngleDirect(&servo_handles[id], angle);
}

// 舵机旋转
void Servo_SetAngleDirect(Servo_HandleTypeDef *servo, uint16_t angle)
{
    servo->angle = angle; // 设置结构体中元素的angle，以后要获取角度大小时可以使用
    uint32_t compare = _angle_to_compare(servo, angle);
    _write_compare(servo, compare);
}

// 获取舵机的角度
/*
uint16_t Servo_GetAngle(Servo_ID id)
{
    return servo_handles[id].angle;
}
*/
static uint32_t _angle_to_compare(Servo_HandleTypeDef *servo, uint16_t angle)
{

    uint32_t compare;
    // 获取时钟频率用于待会的比较器设置
    uint32_t tim_clk = HAL_RCC_GetPCLK1Freq(); // 来自APB1总线的时钟
    if (servo->htim->Instance == TIM1)
    {
        tim_clk = HAL_RCC_GetPCLK2Freq();
    } // 来自APB2总线的时钟
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(servo->htim);     // 自动重装载的值
    compare = (250 + angle * 1000 / 180) * (arr + 1) / 10000; // 此处用标准舵机的角度来计算，否则比较麻烦
    return compare;                                           // 将要设置的比较器的值返回
}

static void _write_compare(Servo_HandleTypeDef *servo, uint32_t compare)
{
    switch (servo->channel)
    {
    case TIM_CHANNEL_1:
        __HAL_TIM_SET_COMPARE(servo->htim, TIM_CHANNEL_1, compare);
        break;
    case TIM_CHANNEL_2:
        __HAL_TIM_SET_COMPARE(servo->htim, TIM_CHANNEL_2, compare);
        break;
    case TIM_CHANNEL_3:
        __HAL_TIM_SET_COMPARE(servo->htim, TIM_CHANNEL_3, compare);
        break;
    case TIM_CHANNEL_4:
        __HAL_TIM_SET_COMPARE(servo->htim, TIM_CHANNEL_4, compare);
        break;
    } // 找到对应的通道设置比较值
}