/*
 * SFM.h
 *
 * Created: 2024-11-04 오전 12:03:51
 *  Author: Byun Young Ju
 */ 


#ifndef SFM_H_
#define SFM_H_

typedef enum {   // 열거형으로 LED 색상코드 정의 
	WHITE = 0b000,
	YELLOW = 0b001,
	RED = 0b011,
	BLUE = 0b110,
	GREEN = 0b101,
	MAGENTA = 0b010,
	CYAN = 0b100,
	BLACK = 0b111
} SFM_COLOR_CODE;

// SFM 초기화
// - USART0 초기화
// - USART1 초기화
// - USART1 원형큐 초기화
void SFM_Init();

// SFM LED 색상 설정
// - 인수: 1번 색상, 2번 색상, 시간
void SFM_LED_Transmit(SFM_COLOR_CODE color1, SFM_COLOR_CODE color2, uint8_t duration);

// SFM 3c3r
// - 인수: 인식 차수
void SFM_3C3R_Transmit(uint8_t nth, uint16_t user_number, uint8_t role_number);

// SFM 1:1
// - 인수: 유저 번호
void SFM_1v1_Transmit(uint16_t n); // 유저넘버는 2바이트에 걸쳐 값을 지정하기때문에 uint16으로.

// SFM 데이터베이스 삭제
void SFM_DB_Delete_Transmit();

// SFM USART 데이터 수신 콜백
// - 인수: 수신된 1바이트 데이터
void SFM_USART_Rx_Callback(uint8_t data);

// SFM 파싱 콜백
void SFM_Parse_Callback(uint8_t cmd, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);

#endif /* SFM_H_ */