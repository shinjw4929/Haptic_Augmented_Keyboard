#include <Keyboard.h>
#include <math.h>

// 스위치 & 핀 정의
#define SWITCH_1 2
#define SWITCH_2 3
#define SWITCH_3 4
#define SWITCH_4 5
#define SWITCH_5 6
#define SWITCH_6 7
#define SWITCH_7 8
#define SWITCH_8 9
#define ACTUATOR_1 DAC0 // 아두이노 DUE DAC 핀

// 키보드 관련
int pins[8] = {SWITCH_1, SWITCH_2, SWITCH_3, SWITCH_4, SWITCH_5, SWITCH_6, SWITCH_7, SWITCH_8};
char keycodes[8] = {'q', 'w', 'e', 'r', 'a', 's', 'd', 'f'};
bool currentSwitchState[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};  // 현재 상태
bool prevSwitchState[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};     // 이전 상태
#define DEBOUNCE_DELAY 20 // 채터링 방지 시간 (밀리초)
unsigned long lastDebounceTime[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// 진동자 제어 관련
#define DEFAULT_FREQUENCY 10
unsigned long lastVibrationUpdate = 0;

// 룩업 테이블
#define TABLE_SIZE 1024
#define MAX_AMPLITUDE 4095 // 12비트 범위: 0 ~ 4095
unsigned short sineWaveTable[TABLE_SIZE];
unsigned short squareWaveTable[TABLE_SIZE];
unsigned short pulseWaveTable[TABLE_SIZE];
unsigned short triangleWaveTable[TABLE_SIZE];
unsigned short sawtoothWaveTable[TABLE_SIZE];

// 게임 상황 관련
unsigned long switch8PressTime = 0;
unsigned long switch8ReleaseTime = 0;
unsigned long placeSettingTime = 0;
char place = 'D'; // 기본 위치

// 진동 이벤트 구조체
struct VibrationEvent {
  unsigned long startTime;
  int vibrationKey;
  int vibrationType;
  int frequency;
  unsigned long duration;
  bool active;
};

#define MAX_VIBRATION_EVENTS 10
VibrationEvent vibrationEvents[MAX_VIBRATION_EVENTS];

void offActuator();
void updateVibrations();
void addVibrationEvent(int key, int type, int freq, unsigned long dur);

void setup() {
  // 스위치 핀 설정
  for (int i = 0; i < 8; i++) {
    pinMode(pins[i], INPUT_PULLUP);
  }
  // 룩업 테이블 초기화
  for (int i = 0; i < TABLE_SIZE; i++) {
    sineWaveTable[i] = 2048 + 2047 * sin(2 * PI * i / TABLE_SIZE);
    if (i < TABLE_SIZE / 2) {
      squareWaveTable[i] = MAX_AMPLITUDE;
      triangleWaveTable[i] = (2 * MAX_AMPLITUDE * i) / TABLE_SIZE;
    }
    else {
      squareWaveTable[i] = 0;
      triangleWaveTable[i] = MAX_AMPLITUDE - (2 * MAX_AMPLITUDE * (i - TABLE_SIZE / 2)) / TABLE_SIZE;
    }
    if (i < TABLE_SIZE / 16) {
      pulseWaveTable[i] = MAX_AMPLITUDE;
    }
    else {
      pulseWaveTable[i] = 0;
    }
    sawtoothWaveTable[i] = (MAX_AMPLITUDE * i) / TABLE_SIZE;
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  // 모든 스위치 상태 읽기
  for (int i = 0; i < 8; i++) {
    currentSwitchState[i] = digitalRead(pins[i]);
  }

  // 스위치 상태 변화 감지, 처리
  for (int i = 0; i < 8; i++) {
    if (currentSwitchState[i] != prevSwitchState[i] && currentMillis - lastDebounceTime[i] > DEBOUNCE_DELAY) {
      lastDebounceTime[i] = currentMillis;
      if (currentSwitchState[i] == LOW) { // 스위치 눌림
        Keyboard.press(keycodes[i]);
        if (i == 7) { // SWITCH_8 (구르기 감지)
          switch8PressTime = currentMillis;
        }
        else if (i == 1) { // SWITCH_2
          if (currentSwitchState[7] == LOW) {
            // SWITCH_2와 SWITCH_8이 동시에 눌림 - 달리기 : 아래에서 처리
          } else {
            int vibType = 0;
            int freq = 0;
            unsigned long duration = 100000;
            switch (place) {
              case 'D':
                vibType = 3; // 펄스파
                freq = 3;
                break;
              case 'W':
                vibType = 5; // 톱니파
                freq = 1;
                break;
              case 'B':
                vibType = 2; // 사각파
                freq = 5;
                break;
              default:
                vibType = 0;
                freq = DEFAULT_FREQUENCY;
                break;
            }
            addVibrationEvent(2, vibType, freq, duration);
          }
        }
      }
      else { // 스위치 떼짐
        Keyboard.release(keycodes[i]);
        if (i == 7) { // SWITCH_8 (구르기)
          switch8ReleaseTime = currentMillis;
          unsigned long duration = switch8ReleaseTime - switch8PressTime;
          if (duration <= 500) { // 0.5초 이내에 떼짐
            addVibrationEvent(8, 2, 50, 800); // 사각파, 50Hz, 800ms
          }
        }
        if (i == 1) { // SWITCH_2 떼짐
          for (int j = 0; j < MAX_VIBRATION_EVENTS; j++) {
            if (vibrationEvents[j].active && vibrationEvents[j].vibrationKey == 2) {
              vibrationEvents[j].active = false;
            }
          }
        }
        if (i == 1 || i == 7) { // SWITCH_2 또는 SWITCH_8 떼짐
          for (int j = 0; j < MAX_VIBRATION_EVENTS; j++) {
            if (vibrationEvents[j].active && vibrationEvents[j].vibrationKey == 9) {
              vibrationEvents[j].active = false;
            }
          }
        }
      }
      prevSwitchState[i] = currentSwitchState[i];
    }
  }

  // SWITCH_2, SWITCH_8 동시에 눌린 경우
  if (currentSwitchState[1] == LOW && currentSwitchState[7] == LOW) {
    int vibType = 0;
    int freq = 0;
    unsigned long duration = 100000;
    switch (place) {
      case 'D':
        vibType = 3; // 펄스파
        freq = 8;
        break;
      case 'W':
        vibType = 5; // 톱니파
        freq = 2;
        break;
      case 'B':
        vibType = 2; // 사각파
        freq = 15;
        break;
      default:
        vibType = 0;
        freq = DEFAULT_FREQUENCY;
        break;
    }
    addVibrationEvent(9, vibType, freq, duration);
  }
 
  // 시리얼 입력 처리
  if (SerialUSB.available()) {
    char ch = SerialUSB.read();
    if (ch == '1') {
      addVibrationEvent(10, 4, 200, 70); // 삼각파, 70Hz, 70ms
    } else if (ch == '2') {
      addVibrationEvent(11, 2, 40, 200); // 사각파, 40Hz, 200ms
    } else if (ch == '3') {
      addVibrationEvent(12, 5, 80, 30); // 톱니파, 80Hz, 30ms
    } else if (ch == 'W') {
      placeSettingTime = currentMillis;
      place = 'W';
    } else if (ch == 'B') {
      placeSettingTime = currentMillis;
      place = 'B';
    }
  }
  if (currentMillis - placeSettingTime > 5000) {
    place = 'D';
  }

  // 진동 업데이트
  updateVibrations();
}

void offActuator() {
  // 모든 진동 이벤트 중지
  for (int i = 0; i < MAX_VIBRATION_EVENTS; i++) {
    vibrationEvents[i].active = false;
  }
}

void addVibrationEvent(int key, int type, int freq, unsigned long dur) {
  for (int i = 0; i < MAX_VIBRATION_EVENTS; i++) {
    if (vibrationEvents[i].active && vibrationEvents[i].vibrationKey == key) {
      return;
    }
  }
  for (int i = 0; i < MAX_VIBRATION_EVENTS; i++) {
    if (!vibrationEvents[i].active) {
      vibrationEvents[i].startTime = millis();
      vibrationEvents[i].vibrationKey = key;
      vibrationEvents[i].vibrationType = type;
      vibrationEvents[i].frequency = freq;
      vibrationEvents[i].duration = dur;
      vibrationEvents[i].active = true;
      break;
    }
  }
}

void updateVibrations() {
  unsigned long currentMillis = millis();
  int combinedSignal = 0; // 모든 진동 신호 합
  for (int i = 0; i < MAX_VIBRATION_EVENTS; i++) {
    if (vibrationEvents[i].active) {
      unsigned long elapsedTime = currentMillis - vibrationEvents[i].startTime;
      if (elapsedTime < vibrationEvents[i].duration) {
        int index = (int)(elapsedTime * vibrationEvents[i].frequency * TABLE_SIZE / 1000) % TABLE_SIZE;
        int signal = 0;

        switch (vibrationEvents[i].vibrationType) {
          case 1:
            signal = sineWaveTable[index];
            break;
          case 2:
            signal = squareWaveTable[index];
            break;
          case 3:
            signal = pulseWaveTable[index];
            break;
          case 4:
            signal = triangleWaveTable[index];
            break;
          case 5:
            signal = sawtoothWaveTable[index];
            break;
          default:
            signal = 0;
            break;
        }
        combinedSignal += signal / MAX_VIBRATION_EVENTS; // 신호 합산, 스케일링
      } else {
        vibrationEvents[i].active = false;
      }
    }
  }

  analogWrite(ACTUATOR_1, combinedSignal);
}
