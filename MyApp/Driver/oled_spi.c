#include "oled_spi.h"
#include "bsp.h"  // [추가] bsp.h 포함하여 상수 정의 사용
#include <string.h>

static uint8_t oled_buffer[1024];

// SW SPI 바이트 전송 함수 (MSB First)
void oled_write_byte(uint8_t dat, uint8_t is_data) {
    if (is_data) OLED_DC_SET();
    else OLED_DC_CLR();

    OLED_CS_CLR();

    for (uint8_t i = 0; i < 8; i++) {
        OLED_SCK_CLR(); // 클럭 LOW

        if (dat & 0x80) OLED_MOSI_SET(); // MSB가 1이면 MOSI HIGH
        else OLED_MOSI_CLR();            // MSB가 0이면 MOSI LOW

        OLED_SCK_SET(); // 클럭 HIGH (데이터 입력)
        dat <<= 1;      // 다음 비트 이동
    }

    OLED_CS_SET();
}

void oled_fill(uint8_t fill_data) {
    memset(oled_buffer, fill_data, sizeof(oled_buffer));
}

void oled_init(void) {
    // 1. 하드웨어 리셋
    OLED_RES_CLR();
    HAL_Delay(100);
    OLED_RES_SET();
    HAL_Delay(100);

    // 2. 초��화 커맨드
    uint8_t cmds[] = {
        0xAE, 0x20, 0x02, 0x81, 0xCF, 0xA1, 0xC8, 0xA8, 0x3F,
        0xD3, 0x00, 0xD5, 0x80, 0xD9, 0xF1, 0xDA, 0x12, 0xDB, 0x40,
        0x8D, 0x14, 0xA4, 0xA6, 0xAF, 0xA0, 0xC0
    };

    for (int i = 0; i < sizeof(cmds); i++) {
        oled_write_byte(cmds[i], 0);
    }

    oled_clear();
    oled_refresh();
}

void oled_clear(void) {
    memset(oled_buffer, 0, sizeof(oled_buffer));
}

void oled_refresh(void) {
    for (uint8_t i = 0; i < 8; i++) {
        oled_write_byte(0xB0 + i, 0);
        oled_write_byte(0x00, 0);
        oled_write_byte(0x10, 0);

        OLED_CS_SET();
        OLED_DC_SET();
        OLED_CS_CLR();

        // 데이터 블록 SW SPI 전송
        for (uint8_t j = 0; j < 128; j++) {
            uint8_t dat = oled_buffer[128 * i + j];
            for (uint8_t k = 0; k < 8; k++) {
                OLED_SCK_CLR();
                if (dat & 0x80) OLED_MOSI_SET();
                else OLED_MOSI_CLR();
                OLED_SCK_SET();
                dat <<= 1;
            }
        }
        OLED_CS_SET();
    }
}

void oled_show_char(uint8_t x, uint8_t y, uint8_t chr) {
    if (x > 122 || y > 56) return;

    uint8_t page = y / 8;
    uint16_t font_ptr = chr * 5;

    for (uint8_t i = 0; i < 5; i++) {
        oled_buffer[x + i + (page * 128)] = font[font_ptr + i];
    }
}

void oled_show_string(uint8_t x, uint8_t y, char *str) {
    while (*str) {
        oled_show_char(x, y, *str);
        x += 6;
        if (x > 122) { x = 0; y += 8; }
        str++;
    }
}

// 1. 개별 픽셀 제어
void oled_draw_pixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x > 127 || y > 63) return; // 화면 밖 예외 처리

    if (color) {
        oled_buffer[x + (y / 8) * 128] |= (1 << (y % 8));
    } else {
        oled_buffer[x + (y / 8) * 128] &= ~(1 << (y % 8));
    }
}

// 2. 선 그리기 (브레젠험 알고리즘)
void oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        oled_draw_pixel(x1, y1, 1);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

// 3. 폰트 확대 출력
void oled_show_scaled_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t scale) {
    uint16_t font_ptr = chr * 5;

    for (uint8_t i = 0; i < 5; i++) {
        uint8_t col = font[font_ptr + i];
        for (uint8_t j = 0; j < 8; j++) {
            if (col & (1 << j)) {
                // scale 크기만큼 픽셀을 반복해서 찍음 (블록화)
                for (uint8_t sx = 0; sx < scale; sx++) {
                    for (uint8_t sy = 0; sy < scale; sy++) {
                        oled_draw_pixel(x + (i * scale) + sx, y + (j * scale) + sy, 1);
                    }
                }
            }
        }
    }
}

// 애니메이션 제어 변수
static uint32_t last_anim_tick = 0;
static uint8_t anim_frame = 0;
#define ANIM_SPEED_MS  100

/**
 * @brief 위쪽 방향 화살표 그리기 (꺾쇠 모양, 속이 비어있음)
 */
static void draw_up_chevron(uint8_t x, uint8_t y) {
    oled_draw_line(x, y, x - 10, y + 10);
    oled_draw_line(x, y, x + 10, y + 10);
    oled_draw_line(x - 10, y + 10, x - 4, y + 10);
    oled_draw_line(x + 10, y + 10, x + 4, y + 10);
    oled_draw_line(x - 4, y + 10, x, y + 6);
    oled_draw_line(x + 4, y + 10, x, y + 6);
}

/**
 * @brief 아래쪽 방향 화살표 그리기 (꺾쇠 모양, 속이 비어있음)
 */
static void draw_down_chevron(uint8_t x, uint8_t y) {
    oled_draw_line(x, y + 10, x - 10, y);
    oled_draw_line(x, y + 10, x + 10, y);
    oled_draw_line(x - 10, y, x - 4, y);
    oled_draw_line(x + 10, y, x + 4, y);
    oled_draw_line(x - 4, y, x, y + 4);
    oled_draw_line(x + 4, y, x, y + 4);
}

// ------------------------------------
// UI 업데이트 함수 (V1 레이아웃 + 개선된 아웃라인 화살표)
// ------------------------------------
void elevator_ui_update(uint8_t floor, uint8_t lift_dir, uint8_t special_state, bool up_btn, bool down_btn) {
    oled_clear();

    // [1] 레이아웃 구분선
    oled_draw_line(60, 0, 60, 63);
    oled_draw_line(60, 48, 127, 48);

    // [2] 좌측: 현재 층수 (큰 폰트)
    oled_show_scaled_char(12, 8, '0' + floor, 7);

    // ------------------------------------
    // [3] 우측 상단: 카 움직임 (개선된 아웃라인 화살표)
    // ------------------------------------
    uint8_t cx = 94;
    bool is_moving = (special_state == BSP_STATE_MOVING);

    // 애니메이션 프레임 업데이트
    if (is_moving && lift_dir != BSP_LIFT_STOP) {
        uint32_t current_tick = HAL_GetTick();
        if (current_tick - last_anim_tick >= ANIM_SPEED_MS) {
            last_anim_tick = current_tick;
            anim_frame = (anim_frame + 1) % 4;
        }
    } else {
        anim_frame = 3;
    }

    if (lift_dir == BSP_LIFT_UP) {
        for (int i = 0; i < 3; i++) {
            uint8_t visible_stages = (anim_frame == 3) ? 3 : (anim_frame + 1);

            if (i < visible_stages) {
                // ==========================================
                // [수정] 역방향 좌표 계산 (아래에서 위로 점등)
                // i=0(맨아래:32) -> i=1(중간:20) -> i=2(맨위:8)
                // ==========================================
                uint8_t y = 32 - (i * 12);
                draw_up_chevron(cx, y);
            }
        }
    }
    else if (lift_dir == BSP_LIFT_DOWN) {
        for (int i = 0; i < 3; i++) {
            uint8_t visible_stages = (anim_frame == 3) ? 3 : (anim_frame + 1);

            if (i < visible_stages) {
                uint8_t y = 8 + (i * 12);
                draw_down_chevron(cx, y);
            }
        }
    }

    // ------------------------------------
    // [4] 우측 하단: 눌린 버튼 및 특수 메시지
    // ------------------------------------
    if (up_btn) {
        oled_draw_line(66, 60, 71, 51);
        oled_draw_line(71, 51, 76, 60);
        oled_draw_line(66, 60, 76, 60);
    }

    if (down_btn) {
        oled_draw_line(112, 51, 122, 51);
        oled_draw_line(122, 51, 117, 60);
        oled_draw_line(117, 60, 112, 51);
    }

    // 특수 메시지 표시
    if (special_state == BSP_STATE_INSPECTION) {
        oled_show_string(82, 55, "INSP");
    }
    else if (special_state == BSP_STATE_MOVING) {
        oled_show_string(82, 55, "MOVE");
    }
    else {
        oled_show_string(82, 55, "NORM");
    }

    oled_refresh();
}