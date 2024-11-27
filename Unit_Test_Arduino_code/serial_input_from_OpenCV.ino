#include <math.h>

#define ACTUATOR_1 DAC0 //아두이노 DUE DAC핀

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
  SerialUSB.begin(9600);

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

  char ch = ' ';
  if (SerialUSB.available()) {
    ch = SerialUSB.read();
    if(ch == 'A'){
      frequency = 10;
      vibrationStartTime = currentMillis;
      vibrationKey = 5; // 진동 키 설정
      isVibrating = true;
    }
    if(ch == 'B'){
      frequency = 25;
      vibrationStartTime = currentMillis;
      vibrationKey = 2; // 진동 키 설정
      isVibrating = true;
    }
  }

 // 1~5 : 파형 변화 6~8 : 진동수 변화
  if (isVibrating) {
    unsigned long elapsedTime = currentMillis - vibrationStartTime;
      switch (vibrationKey){
      case 1:{
        vibrationType = 1;
        if (elapsedTime < 300) {
          vibratorOn = true;
        }
        else{
          offActuator();
        }
        break;
      }
      case 2:{
        vibrationType = 2;
        if (elapsedTime < 300) {
          vibratorOn = true;
        }
        else{
          offActuator();
        }
        break;
      }
      case 3:{
        vibrationType = 3;
        if (elapsedTime < 300) {
          vibratorOn = true;
        }
        else{
          offActuator();
        }
        break;
      }
      case 4:{
        vibrationType = 4;
        if (elapsedTime < 300) {
          vibratorOn = true;
        }
        else{
          offActuator();
        }
        break;
      }
      case 5:{
        vibrationType = 5;
        if (elapsedTime < 300) {
          vibratorOn = true;
        }
        else{
          offActuator();
        }
        break;
      }
      case 6:{
        frequency = 25;
        offActuator();
        break;
      }
      case 7:{
        frequency = 50;
        offActuator();
        break;
      }
      case 8:{
        frequency = 100;
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

