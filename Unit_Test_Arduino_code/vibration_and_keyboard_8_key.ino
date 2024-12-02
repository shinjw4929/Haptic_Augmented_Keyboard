#include <Keyboard.h>
#include <math.h>

//스위치 & 핀
#define SWITCH_1 2
#define SWITCH_2 3
#define SWITCH_3 4
#define SWITCH_4 5
#define SWITCH_5 6
#define SWITCH_6 7
#define SWITCH_7 8
#define SWITCH_8 9
#define ACTUATOR_1 DAC0 //아두이노 DUE DAC핀


//키보드 관련
int pins[8] = {SWITCH_1, SWITCH_2, SWITCH_3, SWITCH_4, SWITCH_5, SWITCH_6, SWITCH_7, SWITCH_8};
char keycodes[8] = {'q', 'w', 'e', 'r', 'a', 's', 'd', 'f'};

bool currentSwitchState[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};  // 눌리기 전
bool prevSwitchState[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};  // 이전 상태

#define DEBOUNCE_DELAY 20 //채터링 방지 시간(millisecond)
unsigned long lastDebounceTime[8] = {0, 0, 0, 0, 0, 0, 0, 0};


//진동자 제어 관련
#define DEFAULT_FREQUENCY 10 //기준 진동수
int vibrationKey = 0;
int vibrationType = 0;
int frequency = DEFAULT_FREQUENCY;

unsigned long vibrationStartTime = 0; //진동 울릴 시간
bool isVibrating = false; //진동 킬 때, 진동 울려야 할 때
bool vibratorOn = false; //진동 함수 호출
unsigned long lastVibrationUpdate = 0;


//파동 룩업 테이블
#define TABLE_SIZE 1024
#define MAX_AMPLITUDE 4095 //12 비트 범위 : 0 ~ 4095
unsigned short sineWaveTable[TABLE_SIZE]; //양수 & 용량 최적화 위해 unsigned short
unsigned short squareWaveTable[TABLE_SIZE];
unsigned short pulseWaveTable[TABLE_SIZE];
unsigned short triangleWaveTable[TABLE_SIZE];
unsigned short sawtoothWaveTable[TABLE_SIZE];


void offActuator();
void updateVibration();

void setup() {
  pinMode(SWITCH_1, INPUT_PULLUP);
  pinMode(SWITCH_2, INPUT_PULLUP);
  pinMode(SWITCH_3, INPUT_PULLUP);
  pinMode(SWITCH_4, INPUT_PULLUP);
  pinMode(SWITCH_5, INPUT_PULLUP);
  pinMode(SWITCH_6, INPUT_PULLUP);
  pinMode(SWITCH_7, INPUT_PULLUP);
  pinMode(SWITCH_8, INPUT_PULLUP);
  pinMode(SWITCH_1, INPUT_PULLUP);
  pinMode(SWITCH_2, INPUT_PULLUP);
  pinMode(SWITCH_3, INPUT_PULLUP);
  pinMode(SWITCH_4, INPUT_PULLUP);
  pinMode(SWITCH_5, INPUT_PULLUP);
  pinMode(SWITCH_6, INPUT_PULLUP);
  pinMode(SWITCH_7, INPUT_PULLUP);
  pinMode(SWITCH_8, INPUT_PULLUP);
  //사인파
  for (int i = 0; i < TABLE_SIZE; i++) {
    sineWaveTable[i] = 2048 + 2047 * sin(2 * PI * i / TABLE_SIZE);
  }
  //사각파
  for (int i = 0; i < TABLE_SIZE; i++) {
    if (i < TABLE_SIZE / 2) {
      squareWaveTable[i] = MAX_AMPLITUDE;
    }
    else {
      squareWaveTable[i] = 0;
    }
  }
  //펄스
  for (int i = 0; i < TABLE_SIZE; i++) {
    if (i < TABLE_SIZE / 16) {
      pulseWaveTable[i] = MAX_AMPLITUDE;
    }
    else {
      pulseWaveTable[i] = 0;
    }
  }
  //삼각파
  for (int i = 0; i < TABLE_SIZE; i++) {
    if (i < TABLE_SIZE / 2) {
      triangleWaveTable[i] = (2 * MAX_AMPLITUDE * i) / TABLE_SIZE;
    }
    else {
      triangleWaveTable[i] = MAX_AMPLITUDE - (2 * MAX_AMPLITUDE * (i - TABLE_SIZE / 2)) / TABLE_SIZE;
    }
  }
  //톱니파
  for (int i = 0; i < TABLE_SIZE; i++) {
    sawtoothWaveTable[i] = (MAX_AMPLITUDE * i) / TABLE_SIZE;
  }
}

void loop() {
  unsigned long currentMillis = millis();
  //핀 전부 읽기
  for (int i = 0; i < 8; i++) {
    currentSwitchState[i] = digitalRead(pins[i]);
  }
  //핀 상태 변화 감지
  for (int i = 0; i < 8; i++) {
    if (currentSwitchState[i] != prevSwitchState[i] && currentMillis - lastDebounceTime[i] > DEBOUNCE_DELAY) {
      lastDebounceTime[i] = currentMillis;
      if (currentSwitchState[i] == LOW) {
        Keyboard.press(keycodes[i]);
        vibrationStartTime = currentMillis;
        vibrationKey = i + 1; // 진동 울릴 키
        isVibrating = true;
      }
      else {
        Keyboard.release(keycodes[i]);
      }
    prevSwitchState[i] = currentSwitchState[i];
    }
  }
 
 // 1~5 : 파형 변화 6~8 : 진동수 변화
  if (isVibrating) {
    unsigned long elapsedTime = currentMillis - vibrationStartTime;
      switch (vibrationKey){
      case 1:{
        vibrationType = 1;
        if (elapsedTime < 1000) {
          vibratorOn = true;
        }
        else{
          offActuator();
        }
        break;
      }
      case 2:{
        vibrationType = 2;
        if (elapsedTime < 1000) {
          vibratorOn = true;
        }
        else{
          offActuator();
        }
        break;
      }
      case 3:{
        vibrationType = 3;
        if (elapsedTime < 1000) {
          vibratorOn = true;
        }
        else{
          offActuator();
        }
        break;
      }
      case 4:{
        vibrationType = 4;
        if (elapsedTime < 1000) {
          vibratorOn = true;
        }
        else{
          offActuator();
        }
        break;
      }
      case 5:{
        vibrationType = 5;
        if (elapsedTime < 1000) {
          vibratorOn = true;
        }
        else{
          offActuator();
        }
        break;
      }
      case 6:{
        frequency = 50;
        offActuator();
        break;
      }
      case 7:{
        frequency = 70;
        offActuator();
        break;
      }
      case 8:{
        frequency = 90;
        offActuator();
        break;
      }
      default: {
        offActuator();
      }
    }
  }
    // 진동자 동작 제어
  if (vibratorOn) {
    updateVibration();
  }
  else {
    analogWrite(ACTUATOR_1, 0);  // 진동자 신호 중단
  }
}

void offActuator(){
  isVibrating = false;
  vibratorOn = false;
  vibrationKey = 0;
  vibrationType = 0;  
}

void updateVibration() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastVibrationUpdate >= 0.01) {  // 업데이트 주기
      int index;
      int signal = 0;
      switch (vibrationType){
        case 1:{ //사인파
          index = (int)((currentMillis - vibrationStartTime) * frequency * TABLE_SIZE / 1000) % TABLE_SIZE;
          signal = sineWaveTable[index];
          break;
        }
        case 2:{ //사각파
          index = (int)((currentMillis - vibrationStartTime) * frequency * TABLE_SIZE / 1000) % TABLE_SIZE;
          signal = squareWaveTable[index];
          break;
        }
        case 3:{ //펄스
          index = (int)((currentMillis - vibrationStartTime) * frequency * TABLE_SIZE / 1000) % TABLE_SIZE;
          signal = pulseWaveTable[index];
          break;
        }
        case 4: { //삼각파
          index = (int)((currentMillis - vibrationStartTime) * frequency * TABLE_SIZE / 1000) % TABLE_SIZE;
          signal = triangleWaveTable[index];
          break;
        }
        case 5: { //톱니파
          index = (int)((currentMillis - vibrationStartTime) * frequency * TABLE_SIZE / 1000) % TABLE_SIZE;
          signal = sawtoothWaveTable[index];
          break;
        }        
        default: {
          signal = 0;
        }
      }
      analogWrite(ACTUATOR_1, signal);
      lastVibrationUpdate = currentMillis;
    }
}

