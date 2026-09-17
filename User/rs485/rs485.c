/**
 * @file rs485.c
 * @brief RS485：USART2 TX=PD5 RX=PD6，DE/RE=PD11（高发低收）
 */
#include "rs485.h"
#include "main.h"
#include <string.h>

UART_HandleTypeDef g_rs485_huart;

static uint8_t  s_rx_ring[RS485_RX_BUF_SIZE];
static volatile uint16_t s_rx_head;
static volatile uint16_t s_rx_tail;
static volatile uint16_t s_frame_len;
static volatile uint8_t  s_frame_ready;
static uint8_t  s_frame_buf[RS485_RX_BUF_SIZE];

static void rs485_gpio_uart_init(uint32_t baudrate)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_USART2_CLK_ENABLE();

  /* PD5 TX / PD6 RX -> AF7 USART2 */
  gpio.Pin = GPIO_PIN_5 | GPIO_PIN_6;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOD, &gpio);

  /* PD11 DE/RE */
  gpio.Pin = GPIO_PIN_11;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Alternate = 0;
  HAL_GPIO_Init(GPIOD, &gpio);

  g_rs485_huart.Instance = USART2;
  g_rs485_huart.Init.BaudRate = baudrate;
  g_rs485_huart.Init.WordLength = UART_WORDLENGTH_8B;
  g_rs485_huart.Init.StopBits = UART_STOPBITS_1;
  g_rs485_huart.Init.Parity = UART_PARITY_NONE;
  g_rs485_huart.Init.Mode = UART_MODE_TX_RX;
  g_rs485_huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  g_rs485_huart.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&g_rs485_huart) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_NVIC_SetPriority(USART2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);

  __HAL_UART_ENABLE_IT(&g_rs485_huart, UART_IT_RXNE);
  __HAL_UART_ENABLE_IT(&g_rs485_huart, UART_IT_IDLE);
}

void rs485_set_tx(void)
{
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
}

void rs485_set_rx(void)
{
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
}

void rs485_init(uint32_t baudrate)
{
  s_rx_head = 0;
  s_rx_tail = 0;
  s_frame_len = 0;
  s_frame_ready = 0;

  rs485_gpio_uart_init(baudrate);
  rs485_set_rx();
}

bool rs485_send(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
  HAL_StatusTypeDef st;

  if ((data == NULL) || (len == 0U))
  {
    return false;
  }

  __HAL_UART_DISABLE_IT(&g_rs485_huart, UART_IT_RXNE);
  __HAL_UART_DISABLE_IT(&g_rs485_huart, UART_IT_IDLE);

  rs485_set_tx();
  for (volatile uint32_t i = 0; i < 2000U; i++)
  {
  }

  st = HAL_UART_Transmit(&g_rs485_huart, (uint8_t *)data, len, timeout_ms);
  rs485_set_rx();

  __HAL_UART_ENABLE_IT(&g_rs485_huart, UART_IT_RXNE);
  __HAL_UART_ENABLE_IT(&g_rs485_huart, UART_IT_IDLE);

  return (st == HAL_OK);
}

uint16_t rs485_read(uint8_t *buf, uint16_t max_len)
{
  uint16_t n;

  if ((buf == NULL) || (max_len == 0U) || (s_frame_ready == 0U))
  {
    return 0U;
  }

  n = s_frame_len;
  if (n > max_len)
  {
    n = max_len;
  }
  memcpy(buf, s_frame_buf, n);
  s_frame_ready = 0U;
  s_frame_len = 0U;
  return n;
}

bool rs485_frame_ready(void)
{
  return (s_frame_ready != 0U);
}

void rs485_clear_frame_ready(void)
{
  s_frame_ready = 0U;
  s_frame_len = 0U;
}

void rs485_irq_handler(void)
{
  uint32_t sr = USART2->SR;
  uint32_t dr;

  if ((sr & USART_SR_RXNE) != 0U)
  {
    uint16_t next;
    dr = USART2->DR;
    next = (uint16_t)((s_rx_head + 1U) % RS485_RX_BUF_SIZE);
    if (next != s_rx_tail)
    {
      s_rx_ring[s_rx_head] = (uint8_t)(dr & 0xFFU);
      s_rx_head = next;
    }
  }

  if ((sr & USART_SR_IDLE) != 0U)
  {
    uint16_t n = 0U;
    dr = USART2->DR;
    (void)dr;

    while ((s_rx_tail != s_rx_head) && (n < RS485_RX_BUF_SIZE))
    {
      s_frame_buf[n++] = s_rx_ring[s_rx_tail];
      s_rx_tail = (uint16_t)((s_rx_tail + 1U) % RS485_RX_BUF_SIZE);
    }
    if (n > 0U)
    {
      s_frame_len = n;
      s_frame_ready = 1U;
    }
  }

  if ((sr & (USART_SR_ORE | USART_SR_FE | USART_SR_NE)) != 0U)
  {
    dr = USART2->DR;
    (void)dr;
  }
}
