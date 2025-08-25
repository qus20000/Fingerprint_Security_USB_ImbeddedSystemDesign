# Fingerprint Recognition Secure USB (ATmega128 Embedded System)

## 프로젝트 개요
본 프로젝트는 대학교 3학년 전공인 임베디드시스템설계 수업의 과제로 시행하였다.
ATmega128 MCU 기반의 임베디드 환경에서 **정전식 지문인식 모듈(SFM-V1.7)**을 제어하여 보안 USB 시스템을 구현한 결과물이다.  
8-bit AVR 마이크로컨트롤러 환경에서 **USART 통신 프로토콜 분석 및 펌웨어 개발**을 통해 MCU를 제어하는 프로그램 작성을 수행하였으며, 하드웨어적으로는**EasyEDA 프로그램으로 ATmega128, Switching Mosfet, 3v3 Regulator, Indicator LED를 포함하여 Schematic 및 PCB를 설계하였으며, Fusion 360 프로그램을 활용한 하우징 3D 모델링 및 FDM 3D프린터를 활용한 프린팅**을 거쳐 완제품 보안 USB를 제작하였다.

---

## 결과물
- **완제품 사진**
  - [product.png]  

- **PCB Schematic**
  - [pcbSchematic.png]    

- **PCB 단면도**
  - [pcb.png]  

---

## 성과 및 경험
- **저사양 8-bit MCU 환경(Atmega128)에서 고속 USART 통신 기반의 지문인식 보안 USB 시스템 구현**  
- **외부 라이브러리 없이 데이터시트 기반 펌웨어 직접 개발**  
- **하드웨어-소프트웨어 통합 개발 경험 (펌웨어, PCB, 하우징, 완제품 제작)**  
- **예산 내에서 개발하기 위해 원가를 절감하기 위한 BOM 관리 및 해외 PCB제작업체 이용 등으로 글로벌 솔루션 활용, 실제 제품 출시를 고려한 손익분기 확인 등을 경험**
- **EDA 프로그램을 통해 PCB Artwork 시행 (완성도 및 신뢰성 있는 하드웨어를 제작, 제품 소형화)**
- **Fusion 360을 통해 제품 하우징 디자인설계 시행 (제품의 완성도 향상)**  
- **QA 검증 경험 (지문 등록, 인증, USB 접근 제어 상황에서 가능한 방해동작 안에서 모두 정상 동작)**  
- **하드웨어 디버깅을 통한 어셈블리 및 레지스터 레벨 CHECK, 오류 수정 및 효율성 향상을 위한 프로그래밍 시행으로 임베디드시스템 개발역량 발전을 경험**
- **USART 통신을 오실로스코프로 확인, 하드웨어 디버거를 통해 Step by Step으로 코드 동작 검증 경험. 레지스터 레벨에서 USART Data Overrun 과 같은 문제를 확인하는 등 문제해결 역량 발전을 경험** 

---

## 시스템 아키텍처
- **MCU**: ATmega128A  
  - 16 MHz XTAL 
  - USART0: Debug Terminal (CoolTerm, 115200 bps)  
  - USART1: Fingerprint SFM-V1.7 Module (SFM-V1.7, 115200 bps)  
- **USB 메모리 제어부**: AO3401A P-MOSFET 기반 VBUS 스위칭을 통해 USB연결 여부를 결정
- **전원부**: RT9080 LDO Regulator (3.3V, 600 mA)  
- **부가 회로**
  - Status LED (전원, 인증 상태 등 표시)  
  - Reset 스위치 (MCU 리셋)  
  - DB Reset 버튼 (지문 등록 데이터 초기화 – 프로토타입 단계에서만 사용)  

---

## 하드웨어 설계
1. **프로토타입**: Atmega128A 개발보드(JMOD128-1) + Breadboard 회로를 통해 초기 프로토타입 회로작동 및 코드 검증을 시행
2. **PCB 설계**: EasyEDA PCBDesign Tool, EMI/EMC 최소화를 고려한 GND CopperFill 및 단거리 라우팅을 통해 동작 신뢰성 중심의 디자인
3. **SMT 제작**: 생산시 원가 절감을 위해 SMD 소자류는 JLCPCB Basic Parts만 선택, Extended Parts(LDO, Crystal, MLCC 등)는 수동 납땜  
4. **하우징 설계**: Fusion360 기반 3D 모델링 및 FDM 3D Printer을 통한 출력 -> 보편적인 케이스디자인. 상·하단 분리형 쉘 구조

---

## 펌웨어 설계
- **언어 및 환경**: C (AVR-GCC), Microchip Studio  
- **통신 프로토콜**: USART Command/Response Packet (고정 8 Byte Frame)  
- **주요 기능**
  - 지문 등록 (3C3R 방식, 3회 반복 등록 절차)  
  - 지문 인증 (1:1 Verification)  
  - LED 색상 피드백 (터치/인증 성공/실패/등록 단계 표시)  
  - USB 접근 제어 (MOSFET 스위칭을 통한 USB전원 제어 : Atmega128 -> PC3 GPIO 제어를 통한 MOSFET 제어)  

## 동작 시퀀스
1. **Touch Interrupt (INT1)** 발생 시 지문 인식 명령 전송  
2. **응답 코드 처리**
   - `0x00`: 인증 성공 → USB VBUS On, Green LED 점등  
   - `0x01`: 인증 실패 → Red LED 점등 후 대기  
   - `0x05`: DB 없음 → 3C3R 절차 시작  
3. **지문 등록**
   - 1차 → 2차 → 3차 등록 후 DB 저장 완료  
4. **USB 사용 가능 상태 진입**

---

## 코드 구조
- **main.c**  
  - 인터럽트 초기화, 지문 인식 루프 제어, MOSFET 제어  
- **SFM.c**  
  - USART0/1 드라이버  
  - 지문인식 모듈 제어 함수 (LED 제어, 3C3R 등록, 1:1 인증, DB Delete)  
  - 응답 파싱 및 상태 플래그 관리  
- **SFM.h**  
  - 데이터 타입 정의 (LED 색상 enum)  
  - 함수 프로토타입 선언  

---

## 참고자료
- SFM-V1.7 Communication Protocol (중국어 원문 -> 직접 번역 및 요약하여 통신 프로토콜 해석)  
- AVR USART Programming Guide  
- [Matrixchung/SFM-V1.7 GitHub] (https://github.com/Matrixchung/SFM-V1.7)  

