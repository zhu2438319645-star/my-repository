# STM32F103C8T6 基础配置与代码生成过程

## 步骤概览

### 1. **创建新项目**

- 打开STM32CubeMX软件
- 选择"New Project"创建新项目
- 在芯片搜索框中输入"STM32F103C8T6"
- 选择正确的芯片型号并创建项目

### 2. **系统核心配置**

- **SYS配置**：
  - Debug模式设置为"Serial Wire"
- **RCC配置**：
  - High Speed Clock (HSE) 选择"Crystal/Ceramic Resonator"

### 3. **时钟树配置**

- 切换到"Clock Configuration"标签页
- 直接将HCLK设置为72MHz，其他配置CubeMX会自动设置达到72MHz的要求

### 4. **外设配置**

- **USART1串口配置**：
  - 在Connectivity中选择USART1
  - Mode设置为Asynchronous（异步模式）
  - 波特率根据蓝牙模块设置为9600

- **NVIC中断配置**：
  - 在System Core的NVIC中
  - 勾选USART1全局中断

- **定时器配置**：
  - **TIM2配置**：
    - Clock Source选择"Internal Clock"
    - Channel1选择"PWM Generation CH1"
    - Prescaler（预分频器）设置为720-1
    - Counter Period（自动重装载值）设置为2000-1
    - 上述设置为了较好地控制舵机

  - **TIM3配置**：
    - Clock Source选择"Internal Clock"
    - Channel1和Channel2选择"PWM Generation CH1/CH2"
    - Prescaler（预分频器）设置为720-1
    - Counter Period（自动重装载值）设置为100-1
    - 上述设置为了较好地控制L298N模块

- **GPIO配置**：
  - PB5-PB8设置为GPIO_Output（用于连接控制模块）
  - PA6, PA7设置为GPIO_Output（用于使能设置）

### 5. **项目生成设置**

- 切换到"Project Manager"标签页
- 设置项目名称
- 选择项目存储路径
- Toolchain/IDE选择"MDK-ARM"

### 6. **引脚分配确认**

- 确认一遍关键引脚分配：
  - USART1: 默认PA9(TX), PA10(RX)
  - TIM2_CH1: 默认PA0
  - TIM3_CH1/CH2: 默认PA6/PA7，即上面提到的使能设置
  - PB5-PB8: GPIO_Output
  - 检查所有引脚配置，避免冲突

### 7. **生成代码**

- 点击右上角的"GENERATE CODE"按钮
- 等待代码生成完成
- 项目文件将自动生成在指定目录中

---

### 8. **编写代码**

#### **先写舵机部分的代码**

#### 舵机的头文件

```c++
#ifndef __SERVO_H
#define __SERVO_H

//养成好习惯，以后可能还会使用c++来混合编写，因此此处设置一下
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
    //初始化
    void Servo_Init(Servo_ID id, TIM_HandleTypeDef *htim, uint32_t channel,uint16_t min_angle,uint16_t max_angle);
    //设置角度
    void Servo_SetAngle(Servo_ID id, uint16_t angle);
    void Servo_SetAngleDirect(Servo_HandleTypeDef *servo, uint16_t angle);
    // uint16_t Servo_GetAngle(Servo_ID id);
    //学的时候有这个，但由于该任务不用获取角度，于是注释掉
    extern Servo_HandleTypeDef servo_handles[SERVO_NUM];

#ifdef __cplusplus
}
#endif

#endif
```

#### 舵机的源文件

```c
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
    servo->angle = angle; // 设置结构体中元素的angle，以后要获取角度大小时可以使用，虽然该任务不会用到
    uint32_t compare = _angle_to_compare(servo, angle);
    _write_compare(servo, compare);//设置比较器里的值
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
```

#### 接下来写电机

#### 电机的头文件

```c++
#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#ifdef __cplusplus//还是跟上面说的一样，养成习惯
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
    void Motor_SetSpeed(Motor_HandleTypeDef *motor, uint8_t speed);             //设置速度
    void Motor_SetDirection(Motor_HandleTypeDef *motor, Motor_Direction dir);   //设置方向
    void Motor_Stop(Motor_HandleTypeDef *motor);                                //停止
    void Motor_Brake(Motor_HandleTypeDef *motor);                               //刹车
    void Motor_Forward(Motor_HandleTypeDef *motor, uint8_t speed);              //前进
    void Motor_Backward(Motor_HandleTypeDef *motor, uint8_t speed);             //后退

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
```

#### 电机的源文件

```c
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
    case MOTOR_FORWARD://前进
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_RESET);
        break;
    case MOTOR_BACKWARD://后退
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_SET);
        break;
    case MOTOR_STOP://停止
        HAL_GPIO_WritePin(motor->IN1_Port, motor->IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->IN2_Port, motor->IN2_Pin, GPIO_PIN_RESET);
        Motor_SetSpeed(motor, 0);
        break;
    case MOTOR_BRAKE://刹车
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

//软转弯，大概一边照常，另一边为原来70%的速度，通过差速转弯
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

//自设转弯强度，0~100
void Robot_ArcTurnLeft(uint8_t base_speed, uint8_t turn_intensity)
{
    if (base_speed > 100)
        base_speed = 100;
    if (turn_intensity > 100)
        turn_intensity = 100;
    //防止超出我们预估的值，超过100就统一设为100

    uint8_t left_speed = base_speed * (100 - turn_intensity) / 100;
    uint8_t right_speed = base_speed;

    if (left_speed < 10)
        left_speed = 0;
    if (right_speed < 10)
        right_speed = 10;
    //由于初始速度不确定，要进行设置，确保安全

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

//与上面的注释一样，只是把左右缓过来，此处不再注释
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

```

#### 最后是main.c主体

由于cubemx自动生成的代码注释太占空间，这里我就将自己写的列出来，就不再把整个代码连cubemx的注释一同写了

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "servo.h"
#include "motor_control.h"
char instructions[32];  //用于接收数据
int re_index = 0;       //用于定位当前输入到哪里
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart1)
  {
    char receivechar = instructions[re_index];
    // 设置好最后为x的话就是设置舵机角度
    if (receivechar == 'x')
    {
      instructions[re_index] = '\0';
      char msg[50];
      int len = sprintf(msg, "Angle: %s\r\n", instructions);
      HAL_UART_Transmit(&huart1, (uint8_t *)msg, len, 100); // 阻塞发送，超时100ms

      uint16_t number = atoi(instructions);
      Servo_SetAngle(SERVO_1, number);               // 设置舵机角度
      memset(instructions, 0, sizeof(instructions)); // 清空数组内容以防妨碍下次接收指令
      re_index = 0;                                  // 重置位置，方便接收下一条指令
    }

    // 最后是m就是设置速度
    else if (receivechar == 'm')
    {
      instructions[re_index] = '\0';
      char msg[50];
      int len = sprintf(msg, "Speed: %s\r\n", instructions);
      HAL_UART_Transmit(&huart1, (uint8_t *)msg, len, 100); // 阻塞发送，超时100ms

      uint8_t number = atoi(instructions);
      Motor_SetSpeed(&motorA, number);
      Motor_SetSpeed(&motorB, number);               // 设置速度
      memset(instructions, 0, sizeof(instructions)); // 清空数组内容以防妨碍下次接收指令
      re_index = 0;                                  // 重置位置，方便接收下一条指令
    }

    // 最后是d就是设置方向
    else if (receivechar == 'd')
    {
      instructions[re_index] = '\0';
      char msg[50];
      int len = sprintf(msg, "Directions: %s\r\n", instructions);
      HAL_UART_Transmit(&huart1, (uint8_t *)msg, len, 100); // 阻塞发送，超时100ms

      uint8_t number = atoi(instructions); // 0直行，1后退，2惯性停下，3刹车
      Motor_SetDirection(&motorA, number);
      Motor_SetDirection(&motorB, number);           // 设置方向
      memset(instructions, 0, sizeof(instructions)); // 清空数组内容以防妨碍下次接收指令
      re_index = 0;                                  // 重置位置，方便接收下一条指令
    }

    // 最后是t就是转向
    else if (receivechar == 't')
    {
      instructions[re_index] = '\0';
      char msg[50];
      int len = sprintf(msg, "Turn: %s\r\n", instructions);
      HAL_UART_Transmit(&huart1, (uint8_t *)msg, len, 100); // 阻塞发送，超时100ms

      uint16_t number = atoi(instructions); // 1原地左转，2原地右转，3软左转，4软右转，5自设角度左转，6自设角度右转
      uint8_t num = number / 1000;
      uint8_t intensity = number - 1000 * num; // 转弯强度
      switch (num)
      {
      case 1:
        Robot_TurnLeft(motorA.speed);
        break;
      case 2:
        Robot_TurnRight(motorB.speed);
        break;
      case 3:
        Robot_SoftTurnLeft(motorA.speed);
        break;
      case 4:
        Robot_SoftTurnRight(motorA.speed);
        break;
      case 5:
        Robot_ArcTurnLeft(motorA.speed, intensity);
        break;
      case 6:
        Robot_ArcTurnRight(motorB.speed, intensity);
        break;
      default:
        break;
      }

      memset(instructions, 0, sizeof(instructions)); // 清空数组内容以防妨碍下次接收指令
      re_index = 0;                                  // 重置位置，方便接收下一条指令
    }
    // 处理数字
    else if (receivechar <= '9' && receivechar >= '0')
    {
      re_index++;
    }

    else // 输入错误直接全部重置
    {
      memset(instructions, 0, sizeof(instructions));
      re_index = 0;
    }
  }
  HAL_UART_Receive_IT(&huart1, (uint8_t *)&instructions[re_index], 1);
}
```

```c
//写在main里面，但是while外面，用于初始化，和一开始的提醒与接收数据
char init_msg[] = "System Ready. Send angle (0-180)x\r\n";
HAL_UART_Transmit(&huart1, (uint8_t *)init_msg, strlen(init_msg), 100);
Servo_Init(SERVO_1, &htim2, TIM_CHANNEL_1, 0, 180);
Motors_InitAll();
HAL_UART_Receive_IT(&huart1, (uint8_t *)&instructions, 1);
```

### 9. **最后将文件放置到对应的地方**

- Inc Src之类的位置，以及将.c.h文件加入工程即可
