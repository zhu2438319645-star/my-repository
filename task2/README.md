# 任务二：通过串口完成单片机与电脑的通信

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

### 3. **时钟树配置**

- 切换到"Clock Configuration"标签页
- 直接将HCLK设置为72MHz，其他配置CubeMX会自动设置

### 4. **外设配置**

- **USART2串口配置**：
  - 在Connectivity中选择USART2
  - Mode设置为Asynchronous（异步模式）

- **NVIC中断配置**：
  - 在System Core的NVIC中
  - 勾选USART2全局中断

- **DMA配置**：
  - 在USART2的DMA Settings中
  - 添加接收通道，用于接收不定长数据

- **GPIO配置**：
  - 设置PC13引脚为GPIO_Output（用于控制小灯亮灭）

### 5. **项目生成设置**

- 切换到"Project Manager"标签页
- 设置项目名称
- 选择项目存储路径
- Toolchain/IDE：选择MDK-ARM

### 6. **引脚分配确认**

- 确认关键引脚分配：
  - PA2: USART2_TX
  - PA3: USART2_RX
  - PC13: GPIO_Output

### 7. **生成代码**

- 点击右上角的"GENERATE CODE"按钮
- 等待代码生成完成
- 如果使用MDK-ARM，会自动生成Keil项目文件

---

### 8. **自己编写代码部分**

先是回调函数这块（因为要写在main前面，所以先展示这一块）

记得前面要加

```c
#include <stdio.h>
#include <string.h>//这两个cubemx不会自动生成，要自己加
```

```C
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)//因为用的是DMA的接收回调函数，所以和任务一的回调函数不一样
{
  if (huart == &huart2)//先判断哪个串口来的信息，其实只开了一个串口，不加也没事（不过养成良好习惯先加）
  {

    if (message[0] == '1' && message[1] == 'x')
    {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_UART_Transmit_DMA(&huart2, (uint8_t*)message, strlen(message));
    }//回显 1
    else if (message[0] == '0' && message[1] == 'x')
    {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_UART_Transmit_DMA(&huart2, (uint8_t*)message, strlen(message));
    }//回显 2
    //对接收的信息进行处理，1x亮灯，0x灭灯
	else
    {
	char err[] = "ERROR INPUT";
	HAL_UART_Transmit_DMA(&huart2, (uint8_t*)err, strlen(err));
	}//既不是1x也不是0x就显示 错误输入
    memset(message,0,sizeof(message));//重置一下接收用的char数组，由于DMA不会自动重置
  }

  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t*)message, 20);
  //再次准备接收数据
}
```

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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t*)message, 20);
  //只加了这一句，用于一开始的接收数据

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

- 用usb转ttl工具直接连接，xcom助手输入1x，0x，24（用于错误测试），测试结果正常
