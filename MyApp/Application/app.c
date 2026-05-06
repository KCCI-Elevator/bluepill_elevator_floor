#include "app.h"

// 전역 변수 설정
uint32_t current_rx_id;
bool up_btn_active = false;
bool down_btn_active = false;
servo_t my_servo;

/**
 * @brief 하드웨어 및 통신 초기화
 */
void app_init() {
  can_init();
  oled_init();
  bspInit();
  servo_init(&my_servo, &htim2, TIM_CHANNEL_1);
}

/**
 * @brief BluePill 메인 제어 태스크
 */
void StartDefaultTask(void *argument)
{
  app_init();

  osDelay(500);

  // PA5~7 핀 상태를 읽어 자신의 층(1, 2, 3)을 식별
  uint8_t my_floor = bspGetLocalFloor();

  oled_clear();
  char startup_msg[16];

  if (my_floor == CAN_FLOOR_UNKNOWN) {
    snprintf(startup_msg, sizeof(startup_msg), "FLOOR: ERR");
  } else {
    snprintf(startup_msg, sizeof(startup_msg), "FLOOR: %d", my_floor);
  }

  oled_show_string(0, 0, startup_msg); // 화면 상단에 층수 표시
  oled_refresh();                      // OLED 하드웨어 업데이트

  osDelay(1000);                       // 1000ms(1초) 동안 대기
  // ------------------------------------------

  // 운행 상태 변수 초기화
  uint8_t current_car_floor = 1;
  uint8_t current_lift_dir = BSP_LIFT_STOP;
  uint8_t special_state = BSP_STATE_NORMAL;

  for(;;) {
    // 1. 버튼 입력 처리 (자신의 층에 맞는 버튼만 동작)
    if (my_floor != CAN_FLOOR_UNKNOWN) {
      uint8_t tx_data[2] = {0};

      // UP 버튼 - 토글 방식 (에지 감지 + 상태 토글)
      if (my_floor != CAN_FLOOR_3 && button_get_pressed(BTN_UP)) {
        tx_data[0] = my_floor;
        tx_data[1] = CAN_CALL_UP;

        if (up_btn_active == false) {
          up_btn_active = true;
          can_transmit(CAN_ID_CALL_REQ, tx_data, 2);
        } else {
          up_btn_active = false;
          can_transmit(CAN_ID_CALL_CANCEL, tx_data, 2);
        }
      }

      // DOWN 버튼 - 토글 방식 (에지 감지 + 상태 토글)
      if (my_floor != CAN_FLOOR_1 && button_get_pressed(BTN_DOWN)) {
        tx_data[0] = my_floor;
        tx_data[1] = CAN_CALL_DOWN;

        if (down_btn_active == false) {
          down_btn_active = true;
          can_transmit(CAN_ID_CALL_REQ, tx_data, 2);
        } else {
          down_btn_active = false;
          can_transmit(CAN_ID_CALL_CANCEL, tx_data, 2);
        }
      }
    }

    // 2. CAN 수신 패킷 처리 (F429 명령 해석)
    if (can_rx_flag == 1) {
      can_rx_flag = 0;

      // 로컬 변수로 처리하여 동기화 문제 해결
      uint32_t rx_id = can_rx_id;
      uint8_t rx_buf[8];
      uint8_t rx_len = can_rx_len;

      // 버퍼 복사
      for (uint8_t i = 0; i < rx_len; i++) {
        rx_buf[i] = can_rx_buf[i];
      }

      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // 수신 LED ON

      if (rx_id == CAN_ID_STATUS) {
        // F429에서 카의 현재 상태를 받음 (층, 방향, 특수상태)
        current_car_floor = rx_buf[0];
        current_lift_dir  = rx_buf[1];
        special_state     = rx_buf[2];
      }
      else if (rx_id == CAN_ID_CALL_CANCEL) {
        // F429가 호출을 처리했으므로 버튼 상태 초기화
        if (rx_buf[0] == my_floor) {
          if (rx_buf[1] == CAN_CALL_UP) {
            up_btn_active = false;
          }
          if (rx_buf[1] == CAN_CALL_DOWN) {
            down_btn_active = false;
          }
        }
      }
      else if (rx_id == CAN_ID_ARRIVED_CHECK) {
        // F429가 도착 확인을 요청했으므로 홀센서 체크
        if (rx_buf[0] == my_floor) {
          // ==========================================
          // 시뮬레이션 환경 (센서 미연결) 대응
          // bspReadHallSensor 검사 생략
          // ==========================================
          // if (bspReadHallSensor(my_floor) == true) {
             uint8_t tx_ack[1] = {my_floor};
             can_transmit(CAN_ID_ARRIVED_ACK, tx_ack, 1);
          // }
        }
      }
      else if (rx_id == CAN_ID_DOOR_CMD) {
        // 도어 제어 명령 처리
        if (rx_buf[0] == my_floor) {
          uint8_t cmd = rx_buf[1];
          uint8_t service_dir = rx_buf[2]; // F429가 지정한 서비스 방향

          if (cmd == DOOR_CMD_OPEN) {
            bspDoorMotorSet(BSP_DOOR_OPEN);

            // 도어가 열렸을 때 버튼 상태 초기화
            if (service_dir == BSP_LIFT_UP) {
              up_btn_active = false;
            }
            else if (service_dir == BSP_LIFT_DOWN) {
              down_btn_active = false;
            }
            else if (service_dir == BSP_LIFT_STOP) {
              up_btn_active = false;
              down_btn_active = false;
            }
          }
          else if (cmd == DOOR_CMD_CLOSE) {
            bspDoorMotorSet(BSP_DOOR_CLOSE);
          }
        }
      }

      osDelay(10);
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // 수신 LED OFF
    }

    // 3. UI 및 애니메이션 업데이트
    elevator_ui_update(current_car_floor, current_lift_dir, special_state, up_btn_active, down_btn_active);

    osDelay(50); //  10ms -> 50ms
  }
}