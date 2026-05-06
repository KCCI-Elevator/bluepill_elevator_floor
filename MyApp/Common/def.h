//
// Created by hiimseoll on 26. 4. 26..
//

#ifndef BLUE_PILL_CAN_COMM_DEF_H
#define BLUE_PILL_CAN_COMM_DEF_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <cmsis_os.h>
#include "main.h"

// define
#define   OK    0
#define   FAIL  1

#define   ON    1
#define   OFF   0

#define   HIGH  1
#define   LOW   0

#define CAN_FLOOR_UNKNOWN 0
#define CAN_FLOOR_1 1
#define CAN_FLOOR_2 2
#define CAN_FLOOR_3 3

// --- CAN 통신 ID 정의  ---
#define CAN_ID_STATUS         0x100  // F429 -> BluePill: 현재 상태 브로드캐스트
#define CAN_ID_CALL_REQ       0x101  // BluePill -> F429: 호출 요청
#define CAN_ID_CALL_CANCEL    0x102  // BluePill -> F429: 호출 취소
#define CAN_ID_ARRIVED_CHECK  0x103  // F429 -> BluePill: 타겟 층 도착, 홀센서 확인 요청
#define CAN_ID_ARRIVED_ACK    0x104  // BluePill -> F429: 홀센서 확인 완료 응답
#define CAN_ID_DOOR_CMD       0x105  // F429 -> BluePill: 문 열림/닫힘 명령 및 버튼 초기화

// --- CAN 통신 데이터 규약 ---
#define CAN_CALL_UP   1
#define CAN_CALL_DOWN 2

#define DOOR_CMD_STOP  0
#define DOOR_CMD_OPEN  1
#define DOOR_CMD_CLOSE 2

#endif //BLUE_PILL_CAN_COMM_DEF_H
