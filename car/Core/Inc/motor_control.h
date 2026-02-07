#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f1xx_hal.h"

    typedef enum
    {
        MOTOR_A = 0,
        MOTOR_B = 1
    } Motor_TypeDef;

    typedef enum
    {
        MOTOR_FORWARD = 0,
        MOTOR_BACKWARD = 1,
        MOTOR_STOP = 2,
        MOTOR_BRAKE = 3
    } Motor_Direction;

    typedef struct
    {
        Motor_TypeDef motor_num;   // 电机编号
        Motor_Direction direction; // 当前方向
        uint8_t speed;             // 当前速度
        GPIO_TypeDef *IN1_Port;    // IN1端口
        uint16_t IN1_Pin;          // IN1引脚
        GPIO_TypeDef *IN2_Port;    // IN2端口
        uint16_t IN2_Pin;          // IN2引脚
        TIM_HandleTypeDef *timer;  // PWM计时器
        uint32_t timer_channel;    // PWM通道
    } Motor_HandleTypeDef;

    // 初始化函数
    void Motor_Init(Motor_HandleTypeDef *motor, Motor_TypeDef motor_num);
    void Motors_InitAll(void);

    // 基础控制函数
    void Motor_SetSpeed(Motor_HandleTypeDef *motor, uint8_t speed);
    void Motor_SetDirection(Motor_HandleTypeDef *motor, Motor_Direction dir);
    void Motor_Stop(Motor_HandleTypeDef *motor);
    void Motor_Brake(Motor_HandleTypeDef *motor);
    void Motor_Forward(Motor_HandleTypeDef *motor, uint8_t speed);
    void Motor_Backward(Motor_HandleTypeDef *motor, uint8_t speed);

    // 转向函数
    void Robot_TurnLeft(uint8_t speed);                                  // 原地左转
    void Robot_TurnRight(uint8_t speed);                                 // 原地右转
    void Robot_SoftTurnLeft(uint8_t speed);                              // 软左转（前进中左转）
    void Robot_SoftTurnRight(uint8_t speed);                             // 软右转（前进中右转）
    void Robot_ArcTurnLeft(uint8_t base_speed, uint8_t turn_intensity);  // 弧线左转
    void Robot_ArcTurnRight(uint8_t base_speed, uint8_t turn_intensity); // 弧线右转

    extern Motor_HandleTypeDef motorA;
    extern Motor_HandleTypeDef motorB;
#ifdef __cplusplus
}
#endif
#endif