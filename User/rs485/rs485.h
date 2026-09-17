/**
 * @file rs485.h
 * @brief RS485：USART2(PD5/PD6) + DE/RE(PD11)
 */
#ifndef RS485_H
#define RS485_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define RS485_BAUDRATE          115200U
#define RS485_RX_BUF_SIZE       256U

void     rs485_init(uint32_t baudrate);
void     rs485_set_tx(void);
void     rs485_set_rx(void);
bool     rs485_send(const uint8_t *data, uint16_t len, uint32_t timeout_ms);
uint16_t rs485_read(uint8_t *buf, uint16_t max_len);
bool     rs485_frame_ready(void);
void     rs485_clear_frame_ready(void);
void     rs485_irq_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* RS485_H */
