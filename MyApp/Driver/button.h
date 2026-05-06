//
// Created by hiimseoll on 26. 4. 27..
//

#ifndef BLUE_PILL_CAN_COMM_BUTTON_H
#define BLUE_PILL_CAN_COMM_BUTTON_H

#include "stm32f1xx_hal.h"
#include "main.h"
#include "def.h"

// 버튼 핀 정의
#define BTN_UP_PORT     GPIOB
#define BTN_UP_PIN      GPIO_PIN_10

#define BTN_DOWN_PORT   GPIOB
#define BTN_DOWN_PIN    GPIO_PIN_11

// 버튼 식별용 Enum
typedef enum {
    BTN_UP = 0,
    BTN_DOWN
} button_type_t;

// 함수 원형
uint8_t button_get_pressed(button_type_t btn);

#endif //BLUE_PILL_CAN_COMM_BUTTON_H
