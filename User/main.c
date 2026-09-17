/**
 * @file main.c
 * @brief Project_Modbus：RS485 + Modbus RTU 从站
 *        USART2 PD5/PD6，DE/RE PD11，HSE 25MHz -> 180MHz，115200 8N1
 */
#include "main.h"
#include "rs485.h"
#include "modbus_rtu.h"

static void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  rs485_init(RS485_BAUDRATE);
  modbus_rtu_init(MODBUS_SLAVE_ADDR);

  while (1)
  {
    modbus_rtu_poll();

    /* 演示：寄存器0随时间变化，便于上位机观察读值是否更新 */
    modbus_rtu_set_reg(0U, (uint16_t)(HAL_GetTick() & 0xFFFFU));
  }
}

/**
 * HSE=25MHz，PLL: M=25 N=360 P=2 -> SYSCLK=180MHz
 * AHB=180，APB1=45，APB2=90
 */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState = RCC_HSE_ON;
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  osc.PLL.PLLM = 25;
  osc.PLL.PLLN = 360;
  osc.PLL.PLLP = RCC_PLLP_DIV2;
  osc.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV4;
  clk.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
