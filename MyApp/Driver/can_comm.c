#include "can_comm.h"

extern CAN_HandleTypeDef hcan;

volatile uint8_t can_rx_flag = 0;
uint8_t can_rx_buf[8];
uint32_t can_rx_id;
uint8_t can_rx_len;

void blink_result(uint8_t is_ok) {
    int count = is_ok ? 2 : 1;
    int delay_ms = is_ok ? 200 : 600;

    for (int i = 0; i < count; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);   // LED ON
        HAL_Delay(delay_ms);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // LED OFF
        HAL_Delay(delay_ms);
    }
}

void can_init(void) {
    CAN_FilterTypeDef canFilter;

    canFilter.FilterBank = 0;
    canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
    canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
    canFilter.FilterIdHigh = 0x0000;
    canFilter.FilterIdLow = 0x0000;
    canFilter.FilterMaskIdHigh = 0x0000;
    canFilter.FilterMaskIdLow = 0x0000;
    canFilter.FilterFIFOAssignment = CAN_RX_FIFO0;
    canFilter.FilterActivation = ENABLE;
    canFilter.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan, &canFilter) != HAL_OK) {
        return;
    }

    if (HAL_CAN_Start(&hcan) != HAL_OK) {
        return;
    }

    if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        return;
    }
}

void can_transmit(uint32_t id, uint8_t *data, uint8_t len) {
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;

    // 빈 사서함이 생길 때까지 아주 잠깐 기다리거나, 없으면 리턴하는 로직이 필요함
    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0) {
        // 사서함이 꽉 찼으면 일단 취소 (보통 노이즈나 버스 과부하 때문)
        return;
    }

    TxHeader.StdId = id;
    TxHeader.IDE   = CAN_ID_STD; // Standard 모드인지 확인!
    TxHeader.RTR   = CAN_RTR_DATA;
    TxHeader.DLC   = len;

    HAL_CAN_AddTxMessage(&hcan, &TxHeader, data, &TxMailbox);
}

// -----------------------------------------------------------------
// [수신부] F429 -> BluePill 데이터 수신 및 파싱
// -----------------------------------------------------------------

/**
 * @brief CAN 메시지 처리 (향후 확장용)
 * @param rx_id 수신한 CAN ID
 * @param rx_data 수신한 데이터 포인터
 * @param rx_len 수신한 데이터 길이
 */
void process_can_rx(uint32_t rx_id, uint8_t *rx_data, uint8_t rx_len) {
    // [예약] 향후 CAN 메시지 전처리 로직 추가 가능
}



#define CAN_RX_COUNT  8
volatile uint32_t can_rx_ids[CAN_RX_COUNT];
volatile uint8_t  can_rx_bufs[CAN_RX_COUNT][8];
volatile uint8_t  can_rx_lens[CAN_RX_COUNT];

// 큐 관리를 위한 인덱스 변수
volatile uint8_t can_rx_head = 0; // 데이터를 넣는 위치
volatile uint8_t can_rx_tail = 0; // 데이터를 꺼내는 위치

/**
 * @brief CAN FIFO0 수신 인터럽트 콜백
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t rx_data[8];

    // FIFO에 메시지가 있는 동안 계속 꺼냅니다.
    while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0)
    {
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, rx_data) == HAL_OK)
        {
            // 다음 저장할 칸 계산
            uint8_t next_head = (can_rx_head + 1) % CAN_RX_COUNT;

            // 큐가 꽉 차지 않았을 때만 저장
            if (next_head != can_rx_tail) {
                can_rx_ids[can_rx_head] = (RxHeader.IDE == CAN_ID_STD) ? RxHeader.StdId : RxHeader.ExtId;
                can_rx_lens[can_rx_head] = RxHeader.DLC;

                for(int i=0; i<RxHeader.DLC; i++) {
                    can_rx_bufs[can_rx_head][i] = rx_data[i];
                }

                can_rx_head = next_head;
                can_rx_flag = 1; // 메인 루프에 알림
            }
        }
    }
}