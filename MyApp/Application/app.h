//
// Created by hiimseoll on 26. 4. 28..
//

#ifndef BLUE_PILL_CAN_COMM_APP_H
#define BLUE_PILL_CAN_COMM_APP_H

#include "hw_def.h"
#include "bsp.h"

extern uint32_t current_rx_id;
extern uint8_t can_rx_len;

void app_init();
void StartDefaultTask(void *argument);

#endif //BLUE_PILL_CAN_COMM_APP_H