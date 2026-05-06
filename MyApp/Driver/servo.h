//
// Created by hiimseoll on 26. 5. 4..
//

#ifndef BLUE_PILL_CAN_COMM_SERVO_H
#define BLUE_PILL_CAN_COMM_SERVO_H

#include "def.h"

typedef enum{
    DOOR_OPEN,
    DOOR_CLOSE
} servo_action_t;

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    servo_action_t state;
    uint16_t min_pulse;
    uint16_t max_pulse;
} servo_t;

void servo_init(servo_t *servo, TIM_HandleTypeDef *htim, uint32_t channel);
void servo_write(servo_t *servo, uint8_t angle);
void set_servo(servo_t *servo, servo_action_t action);
servo_action_t get_servo(servo_t *servo);

#endif //BLUE_PILL_CAN_COMM_SERVO_H
