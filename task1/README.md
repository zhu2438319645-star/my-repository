# 任务一：控制激光模块的开关

# STM32F103C8T6 基础代码生成过程

## 步骤概览

### 1. **创建新项目**

- 打开STM32CubeMX软件
- 选择"New Project"创建新项目
- 在芯片搜索框中输入"STM32F103C8T6"
- 选择正确的芯片型号并创建项目

### 2. **系统核心配置**

- **SYS配置**：
  - Debug模式设置为"Serial Wire"
  - 保持Timebase Source为SysTick（默认）

- **RCC配置**：
  - High Speed Clock (HSE) 选择"Crystal/Ceramic Resonator"

### 3. **时钟树配置**

- 切换到"Clock Configuration"标签页
- 直接将HCLK设置为72MHz，其他配置CubeMX会自动设置达到72MHz的要求

### 4. **外设配置**

- **USART2串口配置**：
  - 在Connectivity中选择USART2
  - Mode设置为Asynchronous（异步模式）
  - 波特率根据蓝牙模块设置（如115200） 我的蓝牙模块为HC-05默认是9600

- **NVIC中断配置**：
  - 在System Core的NVIC中
  - 勾选USART2全局中断

- **GPIO配置**：
  - 设置PA5引脚为GPIO_Output（用于控制激光/LED）
  - 配置输出模式为推挽输出

### 5. **项目生成设置**

- 切换到"Project Manager"标签页
- 设置项目名称："task1"
- 选择项目存储路径
- **重要**：Toolchain/IDE选择"MDK-ARM"

### 6. **引脚分配确认**

- 确认一遍关键引脚分配：
  - PA2: USART2_TX
  - PA3: USART2_RX
  - PA5: GPIO_Output

### 7. **生成代码**

- 点击右上角的"GENERATE CODE"按钮
- 等待代码生成完成
- 如果使用MDK-ARM，会自动生成Keil项目文件

---

### 8. **自己编写代码部分**

先是回调函数这块（因为要写在main前面，所以先展示这一块）

```c
#include <stdio.h>
#include <string.h>//这两个cubemx不会自动生成，要自己加
```

```c
/* USER CODE BEGIN PV */
char message[1];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart2)
  {
    if (message[0] == '1') //如果输入1，就打开激光，并且输出输入的内容（给点反馈会，要不出错时可能不知道哪里有问题）
    {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);//设置引脚为高电平
      HAL_UART_Transmit_IT(&huart2, (uint8_t *)message, 1);//输出输入内容
    }
    else if (message[0] == '0')
    {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);//设置引脚为低电平
      HAL_UART_Transmit_IT(&huart2, (uint8_t *)message, 1);//输出输入内容
    }
  }
  HAL_UART_Receive_IT(&huart2, (uint8_t *)message, 1);//再次设置接收，来接收下次的输入
}
/* USER CODE END 0 */
```

接下来是main

```c
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, (uint8_t *)message, 1);//只加了这一句，其他都是CubeMX自动生成的，用于一开始的接收数据，要写在 MX_USART2_UART_Init(); 的后面，因为要先初始化才可以用
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
```

---

### 9. 最后进行测试

- 用蓝牙连接（这里使用的是手机的serial bluetooth terminal），输入数据进行测试。
