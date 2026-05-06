#include "bsp.h"

// ======================================================================
// F429 보드 전용 BSP 로직
// ======================================================================
#ifdef MCU_F429

bsp_elevator_input_t elevator_input;

/* GPIO port/pin 매핑 */
#define BSP_BTN_FLOOR_1_PORT        0
#define BSP_BTN_FLOOR_1_PIN         0
#define BSP_BTN_FLOOR_2_PORT        0
#define BSP_BTN_FLOOR_2_PIN         1
#define BSP_BTN_FLOOR_3_PORT        0
#define BSP_BTN_FLOOR_3_PIN         2

#define BSP_SENSOR_FLOOR_1_PORT     1
#define BSP_SENSOR_FLOOR_1_PIN      0
#define BSP_SENSOR_FLOOR_2_PORT     1
#define BSP_SENSOR_FLOOR_2_PIN      1
#define BSP_SENSOR_FLOOR_3_PORT     1
#define BSP_SENSOR_FLOOR_3_PIN      2

#define BSP_LIMIT_TOP_PORT          2
#define BSP_LIMIT_TOP_PIN           0
#define BSP_LIMIT_BOTTOM_PORT       2
#define BSP_LIMIT_BOTTOM_PIN        1

#define BSP_DOOR_OPEN_LIMIT_PORT    2
#define BSP_DOOR_OPEN_LIMIT_PIN     2
#define BSP_DOOR_CLOSE_LIMIT_PORT   2
#define BSP_DOOR_CLOSE_LIMIT_PIN    3
#define BSP_DOOR_OBSTACLE_PORT      2
#define BSP_DOOR_OBSTACLE_PIN       4

#define BSP_EMERGENCY_STOP_PORT     2
#define BSP_EMERGENCY_STOP_PIN      5

#define BSP_LIFT_MOTOR_IN1_PORT     3
#define BSP_LIFT_MOTOR_IN1_PIN      0
#define BSP_LIFT_MOTOR_IN2_PORT     3
#define BSP_LIFT_MOTOR_IN2_PIN      1

#define BSP_INPUT_ACTIVE_STATE      HIGH

/**
 * @brief GPIO 입력 핀 읽기
 * @param port_idx 포트 인덱스
 * @param pin_num 핀 번호
 * @return 핀 상태 (true/false)
 */
static bool bspReadInputPin(uint8_t port_idx, uint8_t pin_num) {
    int8_t pin_state;
    pin_state = gpioExtRead(port_idx, pin_num);

    if (pin_state < 0) {
        return false;
    }

    if (BSP_INPUT_ACTIVE_STATE == HIGH) {
        return pin_state == HIGH;
    }

    return pin_state == LOW;
}

/**
 * @brief GPIO 출력 핀 쓰기
 * @param port_idx 포트 인덱스
 * @param pin_num 핀 번호
 * @param state 쓸 상태 (true/false)
 */
static void bspWriteOutputPin(uint8_t port_idx, uint8_t pin_num, bool state) {
    gpioExtWrite(port_idx, pin_num, state ? HIGH : LOW);
}

/**
 * @brief 모터 정지
 * @param in1_port IN1 포트
 * @param in1_pin IN1 핀
 * @param in2_port IN2 포트
 * @param in2_pin IN2 핀
 */
static void bspMotorStop(uint8_t in1_port, uint8_t in1_pin, uint8_t in2_port, uint8_t in2_pin) {
    bspWriteOutputPin(in1_port, in1_pin, false);
    bspWriteOutputPin(in2_port, in2_pin, false);
}

/**
 * @brief 모터 정방향 회전
 * @param in1_port IN1 포트
 * @param in1_pin IN1 핀
 * @param in2_port IN2 포트
 * @param in2_pin IN2 핀
 */
static void bspMotorForward(uint8_t in1_port, uint8_t in1_pin, uint8_t in2_port, uint8_t in2_pin) {
    bspWriteOutputPin(in1_port, in1_pin, true);
    bspWriteOutputPin(in2_port, in2_pin, false);
}

/**
 * @brief 모터 역방향 회전
 * @param in1_port IN1 포트
 * @param in1_pin IN1 핀
 * @param in2_port IN2 포트
 * @param in2_pin IN2 핀
 */
static void bspMotorReverse(uint8_t in1_port, uint8_t in1_pin, uint8_t in2_port, uint8_t in2_pin) {
    bspWriteOutputPin(in1_port, in1_pin, false);
    bspWriteOutputPin(in2_port, in2_pin, true);
}

/**
 * @brief 현재 감지된 층 업데이트
 * 여러 층 센서가 동시에 감지되면 오류로 처리
 */
static void bspUpdateFloorSensor(void) {
    uint8_t detected_count = 0;
    uint8_t detected_floor = 0;

    // 각 층의 센서 상태 확인
    if (bspReadInputPin(BSP_SENSOR_FLOOR_1_PORT, BSP_SENSOR_FLOOR_1_PIN) == true) {
        detected_count++;
        detected_floor = 1;
    }

    if (bspReadInputPin(BSP_SENSOR_FLOOR_2_PORT, BSP_SENSOR_FLOOR_2_PIN) == true) {
        detected_count++;
        detected_floor = 2;
    }

    if (bspReadInputPin(BSP_SENSOR_FLOOR_3_PORT, BSP_SENSOR_FLOOR_3_PIN) == true) {
        detected_count++;
        detected_floor = 3;
    }

    // [수정] 정확히 1개의 센서만 감지되었을 때만 유효
    if (detected_count == 1) {
        elevator_input.floor_valid = true;
        elevator_input.curr_floor = detected_floor;
    }
    else {
        elevator_input.floor_valid = false;
    }
}

/**
 * @brief BSP 초기화 (F429 보드)
 */
void bspInit(void) {
    // [수정] 구조체 완전 초기화
    memset(&elevator_input, 0, sizeof(elevator_input));

    // 초기 상태 설정
    elevator_input.curr_floor = 1;
    elevator_input.current_dir = BSP_LIFT_STOP;
    elevator_input.special_state = BSP_STATE_NORMAL;
    elevator_input.floor_valid = true;
    elevator_input.req_mask = 0;
    elevator_input.arrived_ack = false;

    // 호출 배열 초기화
    for(int i = 0; i < 4; i++) {
      elevator_input.call_up[i] = false;
      elevator_input.call_down[i] = false;
      elevator_input.req_wifi[i] = false;
    }

    // 센서 및 리미트 상태 초기화
    elevator_input.top_limit = false;
    elevator_input.bottom_limit = false;
    elevator_input.door_open_limit = false;
    elevator_input.door_close_limit = false;
    elevator_input.obstacle_detected = false;
    elevator_input.emergency_stop = false;
    elevator_input.motor_over_current = false;

    // 하드웨어 초기화
    hwInit();
}

/**
 * @brief BSP 업데이트 (F429 보드 - 주기적으로 호출)
 * 모든 센��� 및 입력을 읽고 상태를 업데이트
 */
void bspUpdate(void) {
    // 요청 마스크 초기화
    elevator_input.req_mask = 0;

    // 카 내부 버튼 상태 읽기
    if (bspReadInputPin(BSP_BTN_FLOOR_1_PORT, BSP_BTN_FLOOR_1_PIN) == true) {
        elevator_input.req_mask |= (1U << 0);
    }

    if (bspReadInputPin(BSP_BTN_FLOOR_2_PORT, BSP_BTN_FLOOR_2_PIN) == true) {
        elevator_input.req_mask |= (1U << 1);
    }

    if (bspReadInputPin(BSP_BTN_FLOOR_3_PORT, BSP_BTN_FLOOR_3_PIN) == true) {
        elevator_input.req_mask |= (1U << 2);
    }

    // 층 센서 업데이트
    bspUpdateFloorSensor();

    // 리미트 및 센서 상태 읽기
    elevator_input.top_limit = bspReadInputPin(BSP_LIMIT_TOP_PORT, BSP_LIMIT_TOP_PIN);
    elevator_input.bottom_limit = bspReadInputPin(BSP_LIMIT_BOTTOM_PORT, BSP_LIMIT_BOTTOM_PIN);
    elevator_input.door_open_limit = bspReadInputPin(BSP_DOOR_OPEN_LIMIT_PORT, BSP_DOOR_OPEN_LIMIT_PIN);
    elevator_input.door_close_limit = bspReadInputPin(BSP_DOOR_CLOSE_LIMIT_PORT, BSP_DOOR_CLOSE_LIMIT_PIN);
    elevator_input.obstacle_detected = bspReadInputPin(BSP_DOOR_OBSTACLE_PORT, BSP_DOOR_OBSTACLE_PIN);
    elevator_input.emergency_stop = bspReadInputPin(BSP_EMERGENCY_STOP_PORT, BSP_EMERGENCY_STOP_PIN);

    // [주의] motor_over_current는 HAL 드라이버에서 별도로 설정되어야 함
    elevator_input.motor_over_current = false;
}

/**
 * @brief 현재 시간 (밀리초) 반환
 */
uint32_t bspMillis(void) {
    return hwMillis();
}

/**
 * @brief 지정된 시간 동안 대기
 */
void bspDelay(uint32_t delay_ms) {
    hwDelay(delay_ms);
}

/**
 * @brief 엘리베이터 입력 상태 읽기
 * @param input 입력 상태를 저장할 구조체 포인터
 */
void bspElevatorReadInput(bsp_elevator_input_t *input) {
    if (input == NULL) {
        return;
    }

    // [수정] 원자적 복사로 데이터 일관성 보장
    memcpy(input, &elevator_input, sizeof(bsp_elevator_input_t));
}

/**
 * @brief 리프트 모터 제어
 * @param dir 이동 방향 (UP/DOWN/STOP)
 * @param pwm PWM 값 (0-1000)
 */
void bspLiftMotorSet(bsp_lift_dir_t dir, uint16_t pwm) {
    if (pwm == 0 || dir == BSP_LIFT_STOP) {
        bspMotorStop(BSP_LIFT_MOTOR_IN1_PORT, BSP_LIFT_MOTOR_IN1_PIN,
                     BSP_LIFT_MOTOR_IN2_PORT, BSP_LIFT_MOTOR_IN2_PIN);
        return;
    }

    switch (dir) {
        case BSP_LIFT_UP:
            bspMotorForward(BSP_LIFT_MOTOR_IN1_PORT, BSP_LIFT_MOTOR_IN1_PIN,
                            BSP_LIFT_MOTOR_IN2_PORT, BSP_LIFT_MOTOR_IN2_PIN);
            break;
        case BSP_LIFT_DOWN:
            bspMotorReverse(BSP_LIFT_MOTOR_IN1_PORT, BSP_LIFT_MOTOR_IN1_PIN,
                            BSP_LIFT_MOTOR_IN2_PORT, BSP_LIFT_MOTOR_IN2_PIN);
            break;
        case BSP_LIFT_STOP:
        default:
            bspMotorStop(BSP_LIFT_MOTOR_IN1_PORT, BSP_LIFT_MOTOR_IN1_PIN,
                         BSP_LIFT_MOTOR_IN2_PORT, BSP_LIFT_MOTOR_IN2_PIN);
            break;
    }
}

#endif // MCU_F429

// ======================================================================
// BluePill 보드 전용 BSP 로직
// ======================================================================
#ifdef MCU_BLUEPILL

extern void HAL_Delay(uint32_t Delay);
extern uint32_t HAL_GetTick(void);

/**
 * @brief BSP 초기화 (BluePill 보드)
 */
bool bspInit(void) {
    // BluePill 초기화 (필요한 추가 초기화 코드 추가)
    // GPIO, SPI, CAN 등은 main.c 또는 각 모듈에서 이미 초기화됨
    if (adcInit() != true) return false;
    if (ts0224Init() != true) return false;

    return true;
}

/**
 * @brief BSP 업데이트 (BluePill 보드)
 * BluePill은 센서를 직접 읽지 않으므로 특별한 업데이트 불필요
 */
void bspUpdate(void) {
    // BluePill은 주로 CAN을 통해 F429로부터 데이터 수신
    // 로컬 센서 업데이트 필요 시 여기에 추가
}

/**
 * @brief 현재 시간 (밀리초) 반환
 */
uint32_t bspMillis(void) {
    return HAL_GetTick();
}

/**
 * @brief 지정된 시간 동안 대기
 */
void bspDelay(uint32_t delay_ms) {
    HAL_Delay(delay_ms);
}

/**
 * @brief BluePill의 층 식별 (PA5, PA6, PA7 핀 상태로부터)
 * @return 식별된 층 번호 (1, 2, 3) 또는 CAN_FLOOR_UNKNOWN (0)
 *
 * 우선순위: PA5 > PA6 > PA7
 * 여러 핀이 동시에 LOW면 우선순위가 높은 핀의 값을 반환
 */
uint8_t bspGetLocalFloor(void) {
    // PA5 = 0 (LOW) → 1층
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET) {
        return CAN_FLOOR_1;
    }

    // PA6 = 0 (LOW) → 2층
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) {
        return CAN_FLOOR_2;
    }

    // PA7 = 0 (LOW) → 3층
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_RESET) {
        return CAN_FLOOR_3;
    }

    // 어떤 핀도 LOW가 아니면 불명
    return CAN_FLOOR_UNKNOWN;
}

/**
 * @brief 홀센서 읽기 (해당 층의 도착을 감지)
 * @param floor 층 번호 (1, 2, 3)
 * @return true if 해당 층의 홀센서가 감지됨, false otherwise
 *
 * 현재 구현: PB1 사용 (모든 층 공통)
 * 향후: 층별로 다른 핀을 사용하도록 수정 가능
 */
bool bspReadHallSensor(uint8_t floor) {
    // [주의] 현재는 전체 층이 같은 핀(PB1)을 사용
    // 실제 구현에서는 floor 파라미터에 따라 다른 핀을 읽도록 수정 필요

    if (floor == 0) {
        return false;
    }

    // PB1 핀 상태 읽기 (HIGH = 센서 감지)
    return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET);
}

/**
 * @brief 도어 모터 제어
 * @param dir 도어 방향 (OPEN/CLOSE/STOP)
 * @param pwm PWM 값 (현재 미사용, 향후 속도 제어용)
 *
 * PA0 = IN1 (상향)
 * PA1 = IN2 (하향)
 */
extern servo_t my_servo;
void bspDoorMotorSet(bsp_door_dir_t dir) {

    switch (dir) {
    case BSP_DOOR_OPEN:
        // 도어 열기: 서보모터를 180도(또는 열림 위치)로 이동
        set_servo(&my_servo, DOOR_OPEN);
        break;

    case BSP_DOOR_CLOSE:
        // 도어 닫기: 서보모터를 0도(또는 닫힘 위치)로 이동
        set_servo(&my_servo, DOOR_CLOSE);
        break;

    case BSP_DOOR_STOP:
    default:
        break;
    }
}

#endif // MCU_BLUEPILL