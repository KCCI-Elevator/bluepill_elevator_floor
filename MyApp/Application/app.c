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
  //ts0224Init(); // 홀센서 초기화 (bspInit 이후 기준값 설정)
}

// can_comm.c에서 선언된 배열 및 인덱스 변수 외부 참조
extern volatile uint8_t  can_rx_flag;
extern volatile uint32_t can_rx_ids[];
extern volatile uint8_t  can_rx_bufs[][8];
extern volatile uint8_t  can_rx_head;
extern volatile uint8_t  can_rx_tail;

/**
 * @brief BluePill 메인 제어 태스크 (전체 버전)
 */
void StartDefaultTask(void *argument)
{
    // 1. 하드웨어 및 통신 초기화
    app_init();
    osDelay(500);

    // 2. 자신의 위치 식별 (PA5, 6, 7 핀 상태)
    uint8_t my_floor = bspGetLocalFloor();

    // 3. 초기 화면 표시
    oled_clear();
    char startup_msg[16];
    if (my_floor == CAN_FLOOR_UNKNOWN) {
        snprintf(startup_msg, sizeof(startup_msg), "FLOOR: ERR");
    } else {
        snprintf(startup_msg, sizeof(startup_msg), "FLOOR: %d", my_floor);
    }
    oled_show_string(0, 0, startup_msg);
    oled_refresh();
    osDelay(1000);

    // 4. 로컬 운행 상태 변수
    uint8_t current_car_floor = 1;
    uint8_t current_lift_dir = BSP_LIFT_STOP;
    uint8_t special_state = BSP_STATE_NORMAL;
    bool up_btn_active = false;
    bool down_btn_active = false;

    for(;;) {
        // [A] 홀센서 상태 업데이트 (백그라운드에서 상시 수행)
        if (my_floor != CAN_FLOOR_UNKNOWN) {
            ts0224Update();
        }

        // [B] 버튼 입력 처리 (자신의 층에 맞는 버튼만 동작)
        if (my_floor != CAN_FLOOR_UNKNOWN) {
            uint8_t tx_data[2] = {0};

            // UP 버튼 처리 (3층 제외)
            if (my_floor != CAN_FLOOR_3 && button_get_pressed(BTN_UP)) {
                tx_data[0] = my_floor;
                tx_data[1] = CAN_CALL_UP;
                if (!up_btn_active) {
                    up_btn_active = true;
                    can_transmit(CAN_ID_CALL_REQ, tx_data, 2);
                } else {
                    up_btn_active = false;
                    can_transmit(CAN_ID_CALL_CANCEL, tx_data, 2);
                }
            }

            // DOWN 버튼 처리 (1층 제외)
            if (my_floor != CAN_FLOOR_1 && button_get_pressed(BTN_DOWN)) {
                tx_data[0] = my_floor;
                tx_data[1] = CAN_CALL_DOWN;
                if (!down_btn_active) {
                    down_btn_active = true;
                    can_transmit(CAN_ID_CALL_REQ, tx_data, 2);
                } else {
                    down_btn_active = false;
                    can_transmit(CAN_ID_CALL_CANCEL, tx_data, 2);
                }
            }
        }

        // [C] CAN 수신 메시지 큐(배열) 처리
        while (can_rx_tail != can_rx_head)
        {
            uint32_t rx_id = can_rx_ids[can_rx_tail];
            uint8_t* rx_buf = (uint8_t*)can_rx_bufs[can_rx_tail];

            // 1. 상태 정보 수신 (F429 -> BluePill)
            if (rx_id == CAN_ID_STATUS) {
                current_car_floor = rx_buf[0];
                current_lift_dir  = rx_buf[1];
                special_state     = rx_buf[2];
            }
            // 2. 도착 확인 요청 (F429 -> BluePill)
            else if (rx_id == CAN_ID_ARRIVED_CHECK) {
                if (rx_buf[0] == my_floor) {
                    // 큐 방식을 사용하므로 이제 이 조건문이 정확히 실행됩니다.
                    if (bspReadHallSensor(my_floor)) {
                        uint8_t tx_ack[1] = {my_floor};
                        can_transmit(CAN_ID_ARRIVED_ACK, tx_ack, 1);
                    }
                }
            }
            // 3. 호출 취소 확인 (F429가 호출을 수락/처리했을 때)
            else if (rx_id == CAN_ID_CALL_CANCEL) {
                if (rx_buf[0] == my_floor) {
                    if (rx_buf[1] == CAN_CALL_UP)    up_btn_active = false;
                    if (rx_buf[1] == CAN_CALL_DOWN)  down_btn_active = false;
                }
            }
            // 4. 도어 제어 명령
            else if (rx_id == CAN_ID_DOOR_CMD) {
                if (rx_buf[0] == my_floor) {
                    uint8_t cmd = rx_buf[1];
                    uint8_t service_dir = rx_buf[2];

                    if (cmd == DOOR_CMD_OPEN) {
                        bspDoorMotorSet(BSP_DOOR_OPEN);
                        // 도착 시 버튼 램프 끄기
                        if (service_dir == BSP_LIFT_UP)   up_btn_active = false;
                        else if (service_dir == BSP_LIFT_DOWN) down_btn_active = false;
                        else { up_btn_active = false; down_btn_active = false; }
                    }
                    else if (cmd == DOOR_CMD_CLOSE) {
                        bspDoorMotorSet(BSP_DOOR_CLOSE);
                    }
                }
            }

            // 메시지 하나 처리 완료, 다음 인덱스로
            can_rx_tail = (can_rx_tail + 1) % 8; // CAN_RX_MAX 크기 준수
        }

        // 모든 큐를 비웠으므로 플래그 초기화
        can_rx_flag = 0;

        // [D] UI 및 애니메이션 업데이트
        elevator_ui_update(current_car_floor, current_lift_dir, special_state, up_btn_active, down_btn_active);

        osDelay(50);
    }
}