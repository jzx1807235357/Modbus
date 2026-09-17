/**
 * @file modbus_rtu.c
 * @brief 基于 rs485 的 Modbus RTU 从站
 */
#include "modbus_rtu.h"
#include "rs485.h"
#include <string.h>

static uint8_t  s_slave_addr = MODBUS_SLAVE_ADDR;
static uint16_t s_holding[MODBUS_HOLDING_REG_COUNT];
static uint8_t  s_rx[RS485_RX_BUF_SIZE];
static uint8_t  s_tx[RS485_RX_BUF_SIZE];

static uint16_t mb_crc16(const uint8_t *data, uint16_t len)
{
  uint16_t crc = 0xFFFFU;
  uint16_t i;
  uint8_t j;

  for (i = 0U; i < len; i++)
  {
    crc ^= data[i];
    for (j = 0U; j < 8U; j++)
    {
      if ((crc & 0x0001U) != 0U)
      {
        crc = (uint16_t)((crc >> 1) ^ 0xA001U);
      }
      else
      {
        crc >>= 1;
      }
    }
  }
  return crc;
}

static bool mb_crc_ok(const uint8_t *frame, uint16_t len)
{
  uint16_t rx_crc;
  uint16_t calc;

  if (len < 4U)
  {
    return false;
  }
  rx_crc = (uint16_t)frame[len - 2U] | ((uint16_t)frame[len - 1U] << 8);
  calc = mb_crc16(frame, (uint16_t)(len - 2U));
  return (rx_crc == calc);
}

static void mb_send(uint16_t len, bool do_reply)
{
  uint16_t crc;

  if (!do_reply)
  {
    return;
  }
  crc = mb_crc16(s_tx, len);
  s_tx[len++] = (uint8_t)(crc & 0xFFU);
  s_tx[len++] = (uint8_t)(crc >> 8);
  (void)rs485_send(s_tx, len, 100U);
}

static void mb_exception(uint8_t fc, uint8_t code, bool do_reply)
{
  s_tx[0] = s_slave_addr;
  s_tx[1] = (uint8_t)(fc | 0x80U);
  s_tx[2] = code;
  mb_send(3U, do_reply);
}

static void mb_handle_read_holding(const uint8_t *req, uint16_t len, bool do_reply)
{
  uint16_t start;
  uint16_t qty;
  uint16_t i;
  uint16_t out = 0U;

  if (len != 8U)
  {
    mb_exception(MODBUS_FC_READ_HOLDING, 0x03U, do_reply);
    return;
  }

  start = (uint16_t)(((uint16_t)req[2] << 8) | req[3]);
  qty = (uint16_t)(((uint16_t)req[4] << 8) | req[5]);

  if ((qty == 0U) || (qty > 125U) ||
      ((uint32_t)start + qty > MODBUS_HOLDING_REG_COUNT))
  {
    mb_exception(MODBUS_FC_READ_HOLDING, 0x02U, do_reply);
    return;
  }

  s_tx[out++] = s_slave_addr;
  s_tx[out++] = MODBUS_FC_READ_HOLDING;
  s_tx[out++] = (uint8_t)(qty * 2U);
  for (i = 0U; i < qty; i++)
  {
    uint16_t v = s_holding[start + i];
    s_tx[out++] = (uint8_t)(v >> 8);
    s_tx[out++] = (uint8_t)(v & 0xFFU);
  }
  mb_send(out, do_reply);
}

static void mb_handle_write_single(const uint8_t *req, uint16_t len, bool do_reply)
{
  uint16_t addr;
  uint16_t value;

  if (len != 8U)
  {
    mb_exception(MODBUS_FC_WRITE_SINGLE, 0x03U, do_reply);
    return;
  }

  addr = (uint16_t)(((uint16_t)req[2] << 8) | req[3]);
  value = (uint16_t)(((uint16_t)req[4] << 8) | req[5]);

  if (addr >= MODBUS_HOLDING_REG_COUNT)
  {
    mb_exception(MODBUS_FC_WRITE_SINGLE, 0x02U, do_reply);
    return;
  }

  s_holding[addr] = value;
  memcpy(s_tx, req, 6U);
  s_tx[0] = s_slave_addr;
  mb_send(6U, do_reply);
}

static void mb_handle_write_multiple(const uint8_t *req, uint16_t len, bool do_reply)
{
  uint16_t start;
  uint16_t qty;
  uint8_t  byte_cnt;
  uint16_t i;
  uint16_t expect;

  if (len < 9U)
  {
    mb_exception(MODBUS_FC_WRITE_MULTIPLE, 0x03U, do_reply);
    return;
  }

  start = (uint16_t)(((uint16_t)req[2] << 8) | req[3]);
  qty = (uint16_t)(((uint16_t)req[4] << 8) | req[5]);
  byte_cnt = req[6];
  expect = (uint16_t)(9U + byte_cnt);

  if ((len != expect) || (byte_cnt != (uint8_t)(qty * 2U)) ||
      (qty == 0U) || (qty > 123U) ||
      ((uint32_t)start + qty > MODBUS_HOLDING_REG_COUNT))
  {
    mb_exception(MODBUS_FC_WRITE_MULTIPLE, 0x02U, do_reply);
    return;
  }

  for (i = 0U; i < qty; i++)
  {
    s_holding[start + i] =
        (uint16_t)(((uint16_t)req[7U + i * 2U] << 8) | req[8U + i * 2U]);
  }

  s_tx[0] = s_slave_addr;
  s_tx[1] = MODBUS_FC_WRITE_MULTIPLE;
  s_tx[2] = req[2];
  s_tx[3] = req[3];
  s_tx[4] = req[4];
  s_tx[5] = req[5];
  mb_send(6U, do_reply);
}

void modbus_rtu_init(uint8_t slave_addr)
{
  memset(s_holding, 0, sizeof(s_holding));
  s_slave_addr = slave_addr;
  s_holding[0] = 0x0001U;
  s_holding[1] = 0x1234U;
}

void modbus_rtu_set_reg(uint16_t addr, uint16_t value)
{
  if (addr < MODBUS_HOLDING_REG_COUNT)
  {
    s_holding[addr] = value;
  }
}

uint16_t modbus_rtu_get_reg(uint16_t addr)
{
  if (addr < MODBUS_HOLDING_REG_COUNT)
  {
    return s_holding[addr];
  }
  return 0U;
}

void modbus_rtu_poll(void)
{
  uint16_t len;
  uint8_t fc;
  bool do_reply;

  if (!rs485_frame_ready())
  {
    return;
  }

  len = rs485_read(s_rx, sizeof(s_rx));
  if ((len < 4U) || !mb_crc_ok(s_rx, len))
  {
    return;
  }

  if ((s_rx[0] != s_slave_addr) && (s_rx[0] != 0U))
  {
    return;
  }

  do_reply = (s_rx[0] != 0U);
  fc = s_rx[1];

  switch (fc)
  {
  case MODBUS_FC_READ_HOLDING:
    mb_handle_read_holding(s_rx, len, do_reply);
    break;
  case MODBUS_FC_WRITE_SINGLE:
    mb_handle_write_single(s_rx, len, do_reply);
    break;
  case MODBUS_FC_WRITE_MULTIPLE:
    mb_handle_write_multiple(s_rx, len, do_reply);
    break;
  default:
    mb_exception(fc, 0x01U, do_reply);
    break;
  }
}
