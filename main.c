#define F_CPU 16000000UL // 16000000UL // 14745600UL // CPU 주파수 정의
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "SFM.h"

volatile uint8_t Touch_flag = 0;
volatile uint8_t Reset_flag = 0;


// 외부 변수정의 가져오기 
extern uint8_t sfm_flag_recognition;
extern uint8_t sfm_flag_3c3r_1;
extern uint8_t sfm_flag_3c3r_2;
extern uint8_t sfm_flag_3c3r_3;
extern uint8_t sfm_flag_db_delete;
extern uint8_t sfm_flag_led;
extern uint8_t sfm_return_code;
extern uint8_t sfm_flag_3c3r_Success;

uint8_t previousstate = 0;




// USART1 수신 인터럽트 서비스 루틴
ISR(USART1_RX_vect) {
	SFM_USART_Rx_Callback(UDR1);
}


// polling 방식으로 리셋 버튼이 눌리는 것을 감지하는 루틴
void resetbutton(){ 
	uint8_t currentstate = PIND & (1 << PD0);
	
	if (currentstate == 0 && previousstate == 1) { // 버튼이 눌렸을 때만 동작
		//_delay_ms(200); // 디바운싱을 위한 지연
		//if (PIND & (1 << PD0)) { // 다시 한 번 상태 확인 (디바운싱 후)
			
		Reset_flag = 1; // 명령 플래그 설정
		//}
	}
	previousstate = currentstate; // 현재 상태를 이전 상태로 저장
}


// 지문인식 모듈에 터치신호가 들어오는 것을 인터럽트 방식으로 감지
ISR(INT1_vect) {
	if (PIND & (1 << PD1)) { // 터치인터럽트 핀이 HIGH 상태인지 확인
		Touch_flag = 1; // 명령 플래그 설정
	}
}






int main() {
	SFM_Init();
	
	PORTC |= (1 << PC3); // PC3 핀을 HIGH로 설정. 지문인식 결과가 참값일 때 LOW로 변경하면서 모스펫을 작동하기 위해, 미리 Hi로 만들어둔다.
	PORTD |= (1 << PD0); // PD0 핀을 HIGH로 설정. PD0이 버튼에 의해 그라운드로 꽂히면 인터럽트를 하강엣지로 감지하기 위해 미리 Hi로 만들어 둔다.
	DDRC |= (1 << PC3);  // PC3 핀을 출력으로 설정.
	DDRD &= ~(1 << PD1); // PD1를 입력으로 설정.
	DDRD &= ~(1 << PD0); // PD0를 입력으로 설정.

	// 외부 인터럽트 설정
	EIMSK = 0x02;   // INT0과 INT1 인터럽트 활성화 (0x03 = binary 00000011)
	EICRA |= (EICRA & ~(1 << ISC00)) | (1 << ISC01); // 하강엣지에서 INT0 인터럽트 발생
	EICRA |= (1 << ISC11) | (1 << ISC10); // 상승 에지에서 INT1 인터럽트 발생
    sei(); // 전역 인터럽트 허용
	
	Reset_flag = 0;
	
	_delay_ms(500);
	SFM_LED_Transmit(YELLOW, BLACK, 99);
	
	
	while (1) {
		
		_delay_ms(400); // 루프 지연
       /*
		// Reset_flag가 1로 설정되면 데이터베이스 삭제 명령 전송
		if ((Reset_flag == 1) && (Touch_flag == 1)) { // 터치플래그랑 리셋플래그가 같이떴을때,
			// 즉, 리셋버튼을 누르면서 터치를 했을 때에만 리셋되도록 if조건을 걸음.
			Reset_flag = 0; // flag 초기화
			Touch_flag = 0;
			SFM_DB_Delete_Transmit(); // 지문DB 초기화 명령 전송
			while (sfm_flag_db_delete != 1); // 삭제 완료 flag 대기. flag가 0이 아닌값이 될때
			// 위의 while관문을 통과할 수 있다. 즉, 올바른 데이터가 들어올때까지 대기함.
			sfm_flag_db_delete = 0; // flag 초기화
			SFM_LED_Transmit(RED, BLUE, 33); // 삭제완료 후 LED표기. 인수는 RED(011), Blue(110), 33duration
			_delay_ms(1000);
			SFM_LED_Transmit(YELLOW, BLACK, 99);
			// duration 인수를 33보단 99같이 이렇게 길게하면 LED가 두 색상사이를 느리게 페이딩함.
		}
		// 위의 코드는 인터럽트방식으로 사용했던 리셋 코드이며 폴링으로 변경하여 사용을 안하게 되었습니다.
		*/
	   
	   Reset_flag = 0; // reset flag 미리 초기화
	   resetbutton(); // reset flag를 확인하기 위해 함수 호출
	   if (Reset_flag == 1) // reset flag와 touch flag가 동시에 활성화됐을 때 if문 진입 
	   {
		   Reset_flag = 0;
		   SFM_DB_Delete_Transmit();
		   while (sfm_flag_db_delete != 1);
		   sfm_flag_db_delete = 0;
		   SFM_LED_Transmit(CYAN, BLACK, 99);
		   _delay_ms(2200);
		   SFM_LED_Transmit(YELLOW, BLACK, 99);
	   }
	       
		    if (Touch_flag == 1) { // 터치가 들어왔을 때
			Touch_flag = 0; // 플래그 클리어
			SFM_LED_Transmit(BLUE, BLUE, 99); // 터치감지를 의미하는 파란색LED 명령 전송

			SFM_1v1_Transmit(1); // 일단 1대1 비교명령 전송
			while(sfm_flag_recognition != 1); // 1대1비교명령에 대한 응답이 와서 flag가 뜰때까지 대기
			sfm_flag_recognition = 0; // flag 초기화



			//아래 if문은 성공,실패,db없음 에 대한 경우를 차례대로 수행하게 해놨다.
			if (sfm_return_code == 0x00) {  // 5바이트 구간에 0x00인 성공코드가 왔을때!!
				// sfm_return_code는 {0xF5, cmd, p1, p2, p3, p4, 0x00, 0xF5} 에서 P3에 들어온 데이터만을 참조.
				// sfm_return_code에 해당하는 함수 설정은 SFM.c 후반부의 void SFM_Parse_Callback 내에 규칙정의가 돼있다.
				SFM_LED_Transmit(GREEN, BLACK, 50);
				_delay_ms(2300);
				SFM_LED_Transmit(WHITE, WHITE, 33); // 흰색 LED 출력
				break; // while(1)을 완전히 탈출하고 gpio HI 출력쪽으로 빠진다.
				
				
				
				
			}
			else if (sfm_return_code == 0x01) { // 5바이트 구간에 0x01인 실패코드가 왔을때!!
				SFM_LED_Transmit(RED, BLACK, 50);
				_delay_ms(1100);
				SFM_LED_Transmit(YELLOW, BLACK, 99);
				continue; // 함수 꼭대기로 다시 가라.
			}
			else if (sfm_return_code == 0x05) { //  // 5바이트 구간에 0x05인 DB없음 코드가 왔을때!!
				SFM_LED_Transmit(MAGENTA, BLACK, 50); // 보라색 LED를 띄워 지문등록 절차에 들어갔음을 알려줌
				_delay_ms(580);
				SFM_LED_Transmit(MAGENTA, MAGENTA, 50);
				
				
				SFM_3C3R_Transmit(1, 1, 1); // 인수 내 값은 1차3C3R, 유저넘버 1, 권한 1
				while(sfm_flag_3c3r_1 != 1); // 응답대기
				sfm_flag_3c3r_1 = 0; // 응답처리하면 플래그 초기화

				if(sfm_return_code != 0x00) { // 만약 성공인 0x00과 다른 코드가 왔을때
					SFM_DB_Delete_Transmit(); // 지금까지했던 지문등록작업을 초기화.
					while(sfm_flag_db_delete != 1);
					sfm_flag_db_delete = 0;
					SFM_LED_Transmit(RED, BLACK, 50);
					_delay_ms(2300);
					SFM_LED_Transmit(YELLOW, BLACK, 99);
					continue; // 함수 맨위로 다시 올라가서 처음부터 다시 시퀀스를 시행해라.
				}

				SFM_LED_Transmit(GREEN, BLACK, 50);
				_delay_ms(1350);
				SFM_LED_Transmit(MAGENTA, BLACK, 50);
				_delay_ms(580);
				SFM_LED_Transmit(MAGENTA, MAGENTA, 50);
				
				SFM_3C3R_Transmit(2, 0, 0);
				while(sfm_flag_3c3r_2 != 1);
				sfm_flag_3c3r_2 = 0;

				if(sfm_return_code != 0x00) {
					SFM_DB_Delete_Transmit();
					while(sfm_flag_db_delete != 1);
					sfm_flag_db_delete = 0;
					SFM_LED_Transmit(RED, BLACK, 50);
					_delay_ms(2300);
					SFM_LED_Transmit(YELLOW, BLACK, 99);
					continue;
				}

				SFM_LED_Transmit(GREEN, BLACK, 50);
				_delay_ms(1350);
				SFM_LED_Transmit(MAGENTA, BLACK, 50);
				_delay_ms(580);
				SFM_LED_Transmit(MAGENTA, MAGENTA, 50);

				SFM_3C3R_Transmit(3, 0, 0);
				while(sfm_flag_3c3r_3 != 1);
				sfm_flag_3c3r_3 = 0;
				
				if(sfm_return_code != 0x00) {
					SFM_DB_Delete_Transmit();
					while(sfm_flag_db_delete != 1);
					sfm_flag_db_delete = 0;
					SFM_LED_Transmit(RED, BLACK, 50);
					_delay_ms(2300);
					SFM_LED_Transmit(YELLOW, BLACK, 99);
					continue;
				}

				SFM_LED_Transmit(GREEN, BLACK, 20);
				_delay_ms(2300);
				Touch_flag = 0;
				SFM_LED_Transmit(YELLOW, BLACK, 99);
			}
		}
		
	}
	PORTC &= ~(1 << PC3); // break;로 GPIO를 HI로 설정하는 동작
	
	_delay_ms(1000);
}






