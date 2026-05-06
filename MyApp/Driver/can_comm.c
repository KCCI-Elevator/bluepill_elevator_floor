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

void can_transmit(uint32_t id, uint8_t *data, uint8_t len)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;

    TxHeader.StdId = id;                  // 표준 ID
    TxHeader.ExtId = 0x00;                // 확장 ID (사용 안함)
    TxHeader.IDE   = CAN_ID_STD;          // 표준 ID 식별자
    TxHeader.RTR   = CAN_RTR_DATA;        // 데이터 프레임
    TxHeader.DLC   = len;                 // 데이터 길이
    TxHeader.TransmitGlobalTime = DISABLE;

    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0);

    if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, data, &TxMailbox) != HAL_OK) {
        // ==========================================
        // [수정 4] 치명적 버그 수정: while(1); 무한 루프 삭제
        // 통신이 한 번 튀더라도 보드가 죽지 않고 다음 루프를 돌 수 있도록 살려줍니다.
        // ==========================================
        return;
    }
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

/**
 * @brief CAN FIFO0 수신 인터럽트 콜백
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t rx_data[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, rx_data) == HAL_OK)
    {
        if (RxHeader.IDE == CAN_ID_STD) {
            can_rx_id = RxHeader.StdId;
        } else {
            can_rx_id = RxHeader.ExtId;
        }

        can_rx_len = RxHeader.DLC;

        for (uint8_t i = 0; i < RxHeader.DLC; i++) {
            can_rx_buf[i] = rx_data[i];
        }

        for (uint8_t i = RxHeader.DLC; i < 8; i++) {
            can_rx_buf[i] = 0;
        }

        // 수신 후 전처리 함수 호출
        process_can_rx(can_rx_id, can_rx_buf, can_rx_len);

        // 메인 루프(app.c)에 수신 완료 알림
        can_rx_flag = 1;
    }
}