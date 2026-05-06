#ifndef __MAP_BSP__BSP_H_
#define __MAP_BSP__BSP_H_

#include "def.h"

// MCU가 F429인지 BLUEPILL인지 프로젝트 전역 매크로 설정 필요
// #define MCU_F429
#define MCU_BLUEPILL

#ifdef MCU_F429
  #include "hw.h"
  #include "motor.h"
  #include "my_gpio.h"
  #include "my_can.h"
#endif

#ifdef MCU_BLUEPILL
  #include "hw_def.h"
#endif

// --- 열거형 (공통) ---
typedef enum {
    BSP_LIFT_STOP = 0,
    BSP_LIFT_UP,
    BSP_LIFT_DOWN
} bsp_lift_dir_t;

typedef enum {
    BSP_DOOR_STOP = 0,
    BSP_DOOR_OPEN,
    BSP_DOOR_CLOSE
} bsp_door_dir_t;

typedef enum {
    BSP_STATE_NORMAL = 0,
    BSP_STATE_INSPECTION,
    BSP_STATE_MOVING
} bsp_special_state_t;

// --- CAN 관련 정의 (공통) ---
#define CAN_FLOOR_UNKNOWN       0
#define CAN_FLOOR_1             1
#define CAN_FLOOR_2             2
#define CAN_FLOOR_3             3

#define CAN_CALL_UP             1
#define CAN_CALL_DOWN           2

#define CAN_ID_STATUS           0x100
#define CAN_ID_CALL_REQ         0x101
#define CAN_ID_CALL_CANCEL      0x102
#define CAN_ID_ARRIVED_CHECK    0x103
#define CAN_ID_ARRIVED_ACK      0x104
#define CAN_ID_DOOR_CMD         0x105

// [수정] DOOR_CMD는 def.h에서 이미 정의되므로 여기서 재정의하지 않음
// #define DOOR_CMD_OPEN           0x01
// #define DOOR_CMD_CLOSE          0x02

// --- 통합 입력 구조체 (공통, F429 전용) ---
typedef struct {
    bool floor_valid;
    uint8_t curr_floor;
    bsp_lift_dir_t current_dir;

    // 호출 데이터 통일
    uint8_t req_mask;       // 카 내부 키패드 누적 (1<<0: 1층)
    bool call_up[4];        // BluePill CAN 호출 (외부 UP)
    bool call_down[4];      // BluePill CAN 호출 (외부 DOWN)
    bool req_wifi[4];       // ESP-01 WiFi 외부 호출 (원격)

    // 검증 플래그
    bool arrived_ack;       // BluePill로부터 ACK를 받았는지 여부

    // 리미트 및 센서
    bool top_limit;
    bool bottom_limit;
    bool door_open_limit;
    bool door_close_limit;
    bool obstacle_detected;
    bool emergency_stop;
    bool motor_over_current;

    bsp_special_state_t special_state;
} bsp_elevator_input_t;

// --- 함수 원형 (공통) ---
void bspInit(void);
void bspUpdate(void);
uint32_t bspMillis(void);
void bspDelay(uint32_t delay_ms);

// --- F429 전용 함수 ---
#ifdef MCU_F429
void bspElevatorReadInput(bsp_elevator_input_t *input);
void bspLiftMotorSet(bsp_lift_dir_t dir, uint16_t pwm);
#endif

// --- BluePill 전용 함수 ---
#ifdef MCU_BLUEPILL
void bspDoorMotorSet(bsp_door_dir_t dir);
uint8_t bspGetLocalFloor(void);
bool bspReadHallSensor(uint8_t floor);
#endif

#endif //__MAP_BSP__BSP_H_