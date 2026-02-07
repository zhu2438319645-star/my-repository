#include "motor_control.h"
#include "main.h"
extern TIM_HandleTypeDef htim3;

Motor_HandleTypeDef motorA;
Motor_HandleTypeDef motorB;

void Motor_Init(Motor_HandleTypeDef *motor, Motor_TypeDef motor_num)
{
    motor->motor_num = motor_num;
    motor->speed = 0;
    motor->direction = Motor_Stop;
    motor->timer = &htim3;
    if (motor_num == MOTOR_A)
    {
        motor->IN1_Port = GPIOB;
        motor->IN1_Pin = GPIO_PIN_5;
        motor->IN2_Port = GPIOB;
        motor->IN2_Pin = GPIO_PIN_6;
        motor->timer_channel = TIM_CHANNEL_1;
        // 设置初始状态
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_RESET);
    }
    else if (motor_num == MOTOR_B)
    {
        motor->IN1_Port = GPIOB;
        motor->IN1_Pin = GPIO_PIN_7;
        motor->IN2_Port = GPIOB;
        motor->IN2_Pin = GPIO_PIN_8;
        motor->timer_channel = TIM_CHANNEL_2;
        // 设置初始状态
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_RESET);
    }

    // 启动PWM，如果启动失败就直接报错返回
    if (HAL_TIM_PWM_Start(motor->timer, motor->timer_channel) != HAL_OK)
    {
        Error_Handler();
    }
    // 速度设置为0
    __HAL_TIM_SET_COMPARE(motor->timer, motor->timer_channel, 0);
}

// 全部初始化
void Motors_InitAll(void)
{
    Motor_Init(&motorA, MOTOR_A);
    Motor_Init(&motorB, MOTOR_B);
}

// 设置速度
void Motor_SetSpeed(Motor_HandleTypeDef *motor, uint8_t speed)
{
    if (speed > 100)
    {
        speed = 100;
    }
    motor->speed = speed;
    uint8_t arr = __HAL_TIM_GET_AUTORELOAD(motor->timer);
    speed = arr * speed / 100;
    __HAL_TIM_SET_COMPARE(motor->timer, motor->timer_channel, speed);
}

// 设置方向
void Motor_SetDirection(Motor_HandleTypeDef *motor, Motor_Direction dir)
{
    motor->direction = dir;
    switch (dir)
    {
    case MOTOR_FORWARD:
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_RESET);
        break;
    case MOTOR_BACKWARD:
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_SET);
        break;
    case MOTOR_STOP:
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_RESET);
        Motor_SetSpeed(motor, 0);
        break;
    case MOTOR_BRAKE:
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_SET);
        Motor_SetSpeed(motor, 0);
        break;
    default:
        break;
    }
}

// 设置控制函数，便于进行转向
void Motor_Stop(Motor_HandleTypeDef *motor)
{
    Motor_SetDirection(motor, MOTOR_STOP);
    Motor_SetSpeed(motor, 0);
}

void Motor_Brake(Motor_HandleTypeDef *motor)
{
    Motor_SetDirection(motor, MOTOR_BRAKE);
    Motor_SetSpeed(motor, 0);
}
void Motor_Forward(Motor_HandleTypeDef *motor, uint8_t speed)
{
    Motor_SetDirection(motor, MOTOR_FORWARD);
    Motor_SetSpeed(motor, speed);
}
void Motor_Backward(Motor_HandleTypeDef *motor, uint8_t speed)
{
    Motor_SetDirection(motor, MOTOR_BACKWARD);
    Motor_SetSpeed(motor, speed);
}

// 会用到的转向函数

void Robot_TurnLeft(uint8_t speed)
{
    // 左边电机后退，右边电机前进，实现原地左转
    Motor_Backward(&motorA, speed);
    Motor_Forward(&motorB, speed);
}

void Robot_TurnRight(uint8_t speed)
{
    // 左边电机前进，右边电机后退，实现原地右转
    Motor_Forward(&motorA, speed);
    Motor_Backward(&motorB, speed);
}

void Robot_SoftTurnLeft(uint8_t speed)
{
    if (speed > 100)
        speed = 100;

    // 左边电机减速为70%，右边电机保持速度
    uint8_t left_speed = (uint8_t)(speed * 0.7);
    if (left_speed < 30)
        left_speed = 30;

    Motor_Forward(&motorA, left_speed);
    Motor_Forward(&motorB, speed);
}

void Robot_SoftTurnRight(uint8_t speed)
{
    if (speed > 100)
        speed = 100;

    // 右边电机减速为70%，左边电机保持速度
    uint8_t right_speed = (uint8_t)(speed * 0.7);
    if (right_speed < 30)
        right_speed = 30;

    Motor_Forward(&motorA, speed);
    Motor_Forward(&motorB, right_speed);
}

void Robot_ArcTurnLeft(uint8_t base_speed, uint8_t turn_intensity)
{
    if (base_speed > 100)
        base_speed = 100;
    if (turn_intensity > 100)
        turn_intensity = 100;

    uint8_t left_speed = base_speed * (100 - turn_intensity) / 100;
    uint8_t right_speed = base_speed;

    if (left_speed < 10)
        left_speed = 0;
    if (right_speed < 10)
        right_speed = 10;

    if (left_speed == 0)
    {
        Motor_Stop(&motorA);
        Motor_Forward(&motorB, right_speed);
    }
    else
    {
        Motor_Forward(&motorA, left_speed);
        Motor_Forward(&motorB, right_speed);
    }
}

void Robot_ArcTurnRight(uint8_t base_speed, uint8_t turn_intensity)
{
    if (base_speed > 100)
        base_speed = 100;
    if (turn_intensity > 100)
        turn_intensity = 100;

    uint8_t left_speed = base_speed;
    uint8_t right_speed = base_speed * (100 - turn_intensity) / 100;

    if (left_speed < 10)
        left_speed = 10;
    if (right_speed < 10)
        right_speed = 0;

    if (right_speed == 0)
    {
        Motor_Forward(&motorA, left_speed);
        Motor_Stop(&motorB);
    }
    else
    {
        Motor_Forward(&motorA, left_speed);
        Motor_Forward(&motorB, right_speed);
    }
}
