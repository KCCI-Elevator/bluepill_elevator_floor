//
// Created by hiimseoll on 26. 4. 27..
//

#include "button.h"

static uint8_t prev_state_up = 1;
static uint8_t prev_state_down = 1;

/**
 * @brief 버튼 눌림 감지 (개선된 디바운싱)
 * @param btn 버튼 타입 (BTN_UP 또는 BTN_DOWN)
 * @return 버튼이 눌렸으면 1, 아니면 0
 */
uint8_t button_get_pressed(button_type_t btn) {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t* prev_state;

    // 파라미터에 따른 핀 및 상태 변수 할당
    if (btn == BTN_UP) {
        port = BTN_UP_PORT;
        pin = BTN_UP_PIN;
        prev_state = &prev_state_up;
    } else {
        port = BTN_DOWN_PORT;
        pin = BTN_DOWN_PIN;
        prev_state = &prev_state_down;
    }

    // 현재 핀 상태 읽기 (push == 0)
    uint8_t current_state = HAL_GPIO_ReadPin(port, pin);

    // 1에서 0으로의 에지 감지
    if (*prev_state == 1 && current_state == 0) {
        // 첫 번째 디바운싱 대기
        osDelay(20);

        // 첫 번째 확인
        uint8_t confirm_count = 0;
        if (HAL_GPIO_ReadPin(port, pin) == 0) {
            confirm_count++;
        }

        // 추가 확인 (안정성 향상)
        osDelay(5);
        if (HAL_GPIO_ReadPin(port, pin) == 0) {
            confirm_count++;
        }

        osDelay(5);
        if (HAL_GPIO_ReadPin(port, pin) == 0) {
            confirm_count++;
        }

        if (confirm_count >= 2) {
            *prev_state = 0;
            return 1;
        }
    }
    // 0에서 1로의 에지 감지
    else if (*prev_state == 0 && current_state == 1) {
        // 첫 번째 디바운싱 대기
        osDelay(20);

        // 안정화 확인
        uint8_t confirm_count = 0;
        if (HAL_GPIO_ReadPin(port, pin) == 1) {
            confirm_count++;
        }

        osDelay(5);
        if (HAL_GPIO_ReadPin(port, pin) == 1) {
            confirm_count++;
        }

        osDelay(5);
        if (HAL_GPIO_ReadPin(port, pin) == 1) {
            confirm_count++;
        }

        // 3회 중 2회 이상이 HIGH면 버튼 떨어짐 확인
        if (confirm_count >= 2) {
            *prev_state = 1;
        }
    }

    return 0;
}