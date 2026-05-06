#ifndef CAN_COMM_H
#define CAN_COMM_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

extern volatile uint8_t can_rx_flag;
extern uint8_t can_rx_buf[8];
extern uint32_t can_rx_id;
extern uint8_t can_rx_len;

void can_init(void);
void can_transmit(uint32_t id, uint8_t *data, uint8_t len);
void process_can_rx(uint32_t rx_id, uint8_t *rx_data, uint8_t rx_len);
void blink_result(uint8_t is_ok);

#endif // CAN_COMM_H