/*
 * SFM.c
 *
 * Created: 2024-11-04 오전 12:04:14
 *  Author: Byun Young Ju
 */ 
#include <avr/io.h>


#include "SFM.h"

#define BUFFER_SIZE 128

uint8_t sfm_rx_buffer[BUFFER_SIZE];   //수신 버퍼사이즈 설정 초기화
uint8_t sfm_rx_head = 0;  // 원형 큐 사용하려했지만 굳이 안해도될 것 같아서 미사용
uint8_t sfm_rx_tail = 0; // 원형 큐 사용하려했지만 굳이 안해도될 것 같아서 미사용
uint8_t sfm_rx_data_size = 0; // 수신 데이터사이즈 초기화

uint8_t sfm_rx_collect_start = 0;

uint8_t sfm_flag_recognition = 0;
uint8_t sfm_flag_3c3r_1 = 0;
uint8_t sfm_flag_3c3r_2 = 0;
uint8_t sfm_flag_3c3r_3 = 0;
uint8_t sfm_flag_3c3r_Success = 0;
uint8_t sfm_flag_db_delete = 0;
uint8_t sfm_flag_led = 0;
uint8_t sfm_return_code = 0;


void USART0_Transmit(unsigned char *data, uint8_t len) { // usart1 데이터 발신 함수
	for (int i = 0; i < len; i++) {
		while (!(UCSR0A & (1 << UDRE0))); // 전송 준비 대기
		UDR0 = data[i]; // 데이터 전송
		while (!(UCSR0A & (1 << TXC0))); // 전송 완료 대기
		UCSR0A |= (1 << TXC0); // 전송 완료 플래그 클리어
	}
}

void USART1_Transmit(unsigned char *data, uint8_t len) { // usart1 데이터 발신 함수
	for (int i = 0; i < len; i++) {
		while (!(UCSR1A & (1 << UDRE1))); // 전송 준비 대기
		UDR1 = data[i]; // 데이터 전송
		while (!(UCSR1A & (1 << TXC1))); // 전송 완료 대기
		UCSR1A |= (1 << TXC1); // 전송 완료 플래그 클리어
	}
}

void SFM_Init() {
	// USART0 초기화
	UCSR0A = 0x00; // 1배속 전송모드
	UCSR0B = (1 << RXEN0) | (1 << TXEN0); // RX, TX 사용
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // 8 비트 데이터
	UBRR0H = 0x00; // 상위 바이트
	UBRR0L = 0x08; // 하위 바이트
	
	// USART1 초기화
	UCSR1A = 0x00; // 1배속 전송모드
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10); // Data: 8 bit, Parity: Disabled, Asynchronous mode
	UCSR1B = (1<<RXCIE1) | (1<<RXEN1) | (1<<TXEN1); // 0x98   rx tx 사용 , 인터럽트초기화	
	UBRR1H = 0x00; // Set baud rate high
	UBRR1L = 0x08; // Set baud rate low (115200)

	
	// USART1 원형큐 초기화   작성 안했음. 미사용
}



//아래 void SFM~~ 으로 시작하는 코드들 전부 지문인식 모듈에게 전송하는 명령코드들임.
// 아래처럼 함수 뒤 괄호안에 인자들을 넣고, 해당하는 바이트의 command에 해당 인자를 사용하게되면
// 메인 함수에서 이 명령을 전송하기위해 함수를호출할 때, 괄호안에 내가 원하는대로 인자값을 설정하여
// 함께 호출할 수 있음. 

//ex) Main함수 내에서 SFM_LED_Transmit(Red, Blue, 99); 로 함수호출을 하게 되면,
// 아래 command에 내가 설정한 인자가 들어간다. color1은 Red, color2는 Blue, Duration은 99 로. 이렇게 설정하면 빨간색 파란색 LED가 99간격으로 페이딩 한다. 

// 그럼 이 color에 해당하는 것들은 어디에있냐? 바로 SFM.h 에서 정의돼있음.
// SFM_COLOR_CODE 다음에 오는 color1(or2) 의 값은 헤더에서 typedef enum을 이용하여 각각의
// 바이너리값을 넣어주었음. 이 값들은 sfm-v1.7의 커뮤니케이션 프로토콜에서 정의된대로 색상을 설정함.



// SFM.c 에 있는 아래와 같은 함수들은 전부 부연설명이라고 보면 되고, 최초정의는 SFM.h에서 모두 
// 되어있는 것을 확인하고 이해하자.
void SFM_LED_Transmit(SFM_COLOR_CODE color1, SFM_COLOR_CODE color2, uint8_t duration) {
	unsigned char command[8];
	command[0] = 0xF5;
	command[1] = 0xC3;
	command[2] = color1;
	command[3] = color2;
	command[4] = duration;
	command[5] = 0x00;
	command[6] = command[1] ^ command[2] ^ command[3] ^ command[4] ^ command[5]; // command 1~5의 값을 xor연산
	command[7] = 0xF5;

	USART1_Transmit(command, sizeof(command)); // 위 값대로 usart1에 전송
}

void SFM_3C3R_Transmit(uint8_t nth, uint16_t user_number, uint8_t role_number) {
	// nth는 3C3R의 회차로 설정했음. 함수호출할 때 1~3의 값을 넣으면 된다.
	// role_number은 권한인데, 딱히 우리는 사용 안할듯. 그래도 함수호출할 때 01이라도 넣자.

	unsigned char command[8];
	command[0] = 0xF5;
	command[1] = nth;
	command[2] = (user_number >> 8) & 0xFF; 
	 // 유저넘버의 high비트. usernumber은 uint16이기 때문에, 8비트영역에 넣으려면
	 // 위와같이 마스킹이 필요하다. high비트를 설정해야하니까 8번 오른쪽으로 쉬프팅 후 0xFF 마스킹과 &연산.
	command[3] = user_number & 0xFF;  
	// 이건 그냥 0xFF랑 연산하면 0x00, 0xNN 이 나옴. 
	// 여기서 어차피 command한칸의 영역은 1바이트로 제한되어있으니 0xNN만 입력됨
	command[4] = role_number;
	command[5] = 0x00;
	command[6] = command[1] ^ command[2] ^ command[3] ^ command[4] ^ command[5];
	command[7] = 0xF5;
	
	USART1_Transmit(command, sizeof(command));
}

void SFM_1v1_Transmit(uint16_t user_number) {
	unsigned char command[8];
	command[0] = 0xF5;
	command[1] = 0x0B;
	command[2] = (user_number >> 8) & 0xFF;
	command[3] = user_number & 0xFF;
	command[4] = 0x00;
	command[5] = 0x00;
	command[6] = command[1] ^ command[2] ^ command[3] ^ command[4] ^ command[5];
	command[7] = 0xF5;
	
	USART1_Transmit(command, sizeof(command));	
}

void SFM_DB_Delete_Transmit() {
	unsigned char command[8];
	command[0] = 0xF5;
	command[1] = 0x05;
	command[2] = 0x00;
	command[3] = 0x00;
	command[4] = 0x00;
	command[5] = 0x00;
	command[6] = command[1] ^ command[2] ^ command[3] ^ command[4] ^ command[5];
	command[7] = 0xF5;
		
	USART1_Transmit(command, sizeof(command));
}


void SFM_USART_Rx_Callback(uint8_t data) {
	// 조건
	// - 0xF5로 시작하면 버퍼에 데이터 모으기 시작 (sfm_rx_collect_start = 1)
	// - 데이터 모으기 시작한 상태에서 0xF5가 들어오면 데이터 모으기 종료 (sfm_rx_collect_start = 0)
	// - 모인 데이터 길이가 8이고 체크섬이 일치하면 파싱 시작
	
	if(sfm_rx_collect_start == 0) {
		
		 // 0xF5로 시작하면 버퍼에 데이터 모으기 시작
		 if(data == 0xF5) {
			 sfm_rx_buffer[sfm_rx_data_size] = data;
			 sfm_rx_data_size += 1;
			 sfm_rx_collect_start = 1;
		 }
	}
	else if(sfm_rx_collect_start == 1) {
		sfm_rx_buffer[sfm_rx_data_size] = data;
		sfm_rx_data_size += 1;
		
		// 0xF5가 들어오면 데이터 모으기 종료
		if(data == 0xF5) {
			sfm_rx_collect_start = 0;
			// 모인 데이터 길이가 8이고 체크섬이 일치하면 파싱 시작
			uint8_t checksum = sfm_rx_buffer[1] 
							^ sfm_rx_buffer[2]
							^ sfm_rx_buffer[3]
							^ sfm_rx_buffer[4]
							^ sfm_rx_buffer[5];
			if(sfm_rx_data_size == 8 && checksum == sfm_rx_buffer[6]) {
				SFM_Parse_Callback(sfm_rx_buffer[1], 
					sfm_rx_buffer[2], sfm_rx_buffer[3], sfm_rx_buffer[4], sfm_rx_buffer[5]);
			
			}
			sfm_rx_data_size = 0;
		}
	}
}

void SFM_Parse_Callback(uint8_t cmd, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4) {
	unsigned char parsed_data[8] = {0xF5, cmd, p1, p2, p3, p4, 0x00, 0xF5};
	parsed_data[6] = cmd ^ p1 ^ p2 ^ p3 ^ p4; // Checksum 계산

	switch(cmd) {   // Parse callback 결과에서, 2번째 바이트인 cmd값에 대한 switch문
		// 지문인식 결과
		case 0x0B:
		sfm_return_code = p3;  // return_code에 바로 p3을 리턴
		sfm_flag_recognition = 1;
		USART0_Transmit(parsed_data, sizeof(parsed_data)); // USART0으로 전송
		break;

		// 3C3R 1 응답
		case 0x01:
		sfm_return_code = p3;
		sfm_flag_3c3r_1 = 1;
		USART0_Transmit(parsed_data, sizeof(parsed_data)); // USART0으로 전송
		break;

		// 3C3R 2 응답
		case 0x02:
		sfm_return_code = p3;
		sfm_flag_3c3r_2 = 1;
		USART0_Transmit(parsed_data, sizeof(parsed_data)); // USART0으로 전송
		break;
		
		// 3C3R 3 응답
		case 0x03:
		sfm_return_code = p3;
		sfm_flag_3c3r_3 = 1;
		USART0_Transmit(parsed_data, sizeof(parsed_data)); // USART0으로 전송
		break;
		
		// SFM Database delete 응답
		case 0x05:
		sfm_flag_db_delete = 1;
		USART0_Transmit(parsed_data, sizeof(parsed_data)); // USART0으로 전송
		break;
		
		// LED 제어요청 응답
		case 0xC3:
		break;
		
	
	}
}
