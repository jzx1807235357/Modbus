/**
 * @file modbus_rtu.h
 * @brief 基于 rs485 的简易 Modbus RTU 从站
 */
#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define MODBUS_SLAVE_ADDR           0x01U
#define MODBUS_HOLDING_REG_COUNT    64U

#define MODBUS_FC_READ_HOLDING      0x03U
#define MODBUS_FC_WRITE_SINGLE      0x06U
#define MODBUS_FC_WRITE_MULTIPLE    0x10U

void     modbus_rtu_init(uint8_t slave_addr);
void     modbus_rtu_poll(void);
void     modbus_rtu_set_reg(uint16_t addr, uint16_t value);
uint16_t modbus_rtu_get_reg(uint16_t addr);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_RTU_H */
