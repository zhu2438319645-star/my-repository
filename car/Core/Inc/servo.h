#ifndef __SERVO_H
#define __SERVO_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"
#include "tim.h"

// 舵机数量
#define SERVO_NUM 1

    typedef enum
    {
        SERVO_1 = 0
    } Servo_ID;

    // 舵机的结构体，实际上用到的舵机只有一个（所以参数也一样就没必要写结构体），但为了规范还是写一个结构体
    typedef struct
    {
        Servo_ID id;
        TIM_HandleTypeDef *htim;
        uint32_t channel;
        uint16_t angle;
        uint16_t min_angle;
        uint16_t max_angle;

    } Servo_HandleTypeDef;

    // 写关于舵机使用的各种函数
    void Servo_Init(Servo_ID id, TIM_HandleTypeDef *htim, uint32_t channel,
                    uint16_t min_angle, uint16_t max_angle);
    void Servo_SetAngle(Servo_ID id, uint16_t angle);
    void Servo_SetAngleDirect(Servo_HandleTypeDef *servo, uint16_t angle);
    // uint16_t Servo_GetAngle(Servo_ID id);
    extern Servo_HandleTypeDef servo_handles[SERVO_NUM];
#ifdef __cplusplus
}
#endif
#endif