#ifndef __OLED_SPI_H
#define __OLED_SPI_H

#include "stm32f1xx_hal.h"
#include "def.h"
#include "font.h"

// 제어 핀 정의 (변경된 핀 맵 적용)
#define OLED_CS_PORT   GPIOA
#define OLED_CS_PIN    GPIO_PIN_8

#define OLED_DC_PORT   GPIOB
#define OLED_DC_PIN    GPIO_PIN_15

#define OLED_RES_PORT  GPIOB
#define OLED_RES_PIN   GPIO_PIN_14

// SW SPI용 클럭 및 데이터 핀
#define OLED_SCK_PORT  GPIOB
#define OLED_SCK_PIN   GPIO_PIN_12

#define OLED_MOSI_PORT GPIOB
#define OLED_MOSI_PIN  GPIO_PIN_13

// 핀 제어 매크로
#define OLED_CS_SET()  HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_SET)
#define OLED_CS_CLR()  HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_RESET)

#define OLED_DC_SET()  HAL_GPIO_WritePin(OLED_DC_PORT, OLED_DC_PIN, GPIO_PIN_SET)
#define OLED_DC_CLR()  HAL_GPIO_WritePin(OLED_DC_PORT, OLED_DC_PIN, GPIO_PIN_RESET)

#define OLED_RES_SET() HAL_GPIO_WritePin(OLED_RES_PORT, OLED_RES_PIN, GPIO_PIN_SET)
#define OLED_RES_CLR() HAL_GPIO_WritePin(OLED_RES_PORT, OLED_RES_PIN, GPIO_PIN_RESET)

#define OLED_SCK_SET() HAL_GPIO_WritePin(OLED_SCK_PORT, OLED_SCK_PIN, GPIO_PIN_SET)
#define OLED_SCK_CLR() HAL_GPIO_WritePin(OLED_SCK_PORT, OLED_SCK_PIN, GPIO_PIN_RESET)

#define OLED_MOSI_SET() HAL_GPIO_WritePin(OLED_MOSI_PORT, OLED_MOSI_PIN, GPIO_PIN_SET)
#define OLED_MOSI_CLR() HAL_GPIO_WritePin(OLED_MOSI_PORT, OLED_MOSI_PIN, GPIO_PIN_RESET)

void oled_init(void);
void oled_clear(void);
void oled_fill(uint8_t fill_data);
void oled_refresh(void);
void oled_write_byte(uint8_t dat, uint8_t is_data);
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr);
void oled_show_string(uint8_t x, uint8_t y, char *str);

// 그래픽 및 UI 그리기 함수 원형
void oled_draw_pixel(uint8_t x, uint8_t y, uint8_t color);
void oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void oled_show_scaled_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t scale);

// 엘리베이터 전용 디스플레이 업데이트 (bsp_lift_dir_t, bsp_special_state_t 사용)
void elevator_ui_update(uint8_t floor, uint8_t lift_dir, uint8_t special_state, bool up_btn, bool down_btn);

#endif