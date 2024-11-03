#include <Keyboard.h>
#include <math.h>

#define PIN_SWITCH_1 3
#define PIN_SWITCH_2 4
#define PIN_SWITCH_3 5
#define PIN_SWITCH_4 6
#define VIBRATOR_PIN DAC1

#define KEYCODE_1 'q'
#define KEYCODE_2 'w'
#define KEYCODE_3 'e'
#define KEYCODE_4 'r'

//키보드 관련
bool switchState[4] = {HIGH, HIGH, HIGH, HIGH};  // HIGH로 초기화 (대기 상태)
bool lastSwitchState[4] = {HIGH, HIGH, HIGH, HIGH};  // 이전 상태 저장
#define DEBOUNCE_DELAY 20
unsigned long lastDebounceTime[4] = {0, 0, 0, 0};

//진동자 제어 관련
unsigned long vibrationStartTime = 0; //진동 울릴 시간
bool isVibrating = false; // 진동 패턴이 진행 중인지 여부
bool vibratorOn = false;  // 진동자가 현재 켜져 있는지 여부

int vibrate_key = -1; //진동 울리는 키
unsigned long lastVibrationUpdate = 0;


//진동 파형 관련
//룩업 테이블
#define TABLE_SIZE 1024
uint16_t sineTable[TABLE_SIZE];
uint16_t squareWaveTable[TABLE_SIZE];
uint16_t pulseWaveTable[TABLE_SIZE];
uint16_t customWave[TABLE_SIZE];
//주파수
int frequency = 10;
int dullFrequency = 5;
int sharpFrequency = 100;

//초기화
void setup() {
  pinMode(PIN_SWITCH_1, INPUT_PULLUP);
  pinMode(PIN_SWITCH_2, INPUT_PULLUP);
  pinMode(PIN_SWITCH_3, INPUT_PULLUP);
  pinMode(PIN_SWITCH_4, INPUT_PULLUP);

  Keyboard.begin();

  // 사인파 테이블 초기화
  for (int i = 0; i < TABLE_SIZE; i++) {
    sineTable[i] = 2048 + 2047 * sin(2 * PI * i / TABLE_SIZE); //12비트 범위
  }
  // 사각파 테이블 초기화
  for (int i = 0; i < TABLE_SIZE; i++) {
    if (i < TABLE_SIZE / 2) {
      squareWaveTable[i] = 4095;  // 최대값
    } else {
      squareWaveTable[i] = 0;     // 최소값
    }
  }
  // 펄스 파형 테이블 초기화
  for (int i = 0; i < TABLE_SIZE; i++) {
    if (i < TABLE_SIZE / 16) {
      pulseWaveTable[i] = 4095;  // 펄스 지속 시간 (테이블의 1/16)
    } else {
      pulseWaveTable[i] = 0;     // 나머지 시간은 낮은 상태
    }
  }
    // 사용자 정의 파형 초기화
  for (int i = 0; i < TABLE_SIZE; i++) {
    // 감쇄되는 사인파
    customWave[i] = 2048 + (2047 * sin(2 * PI * i / TABLE_SIZE)) * exp(-0.01 * i);
  }
}

void loop() {
  // 현재 시간을 가져온다
  unsigned long currentMillis = millis();


  int pins[4] = {PIN_SWITCH_1, PIN_SWITCH_2, PIN_SWITCH_3, PIN_SWITCH_4};
  char keycodes[4] = {KEYCODE_1, KEYCODE_2, KEYCODE_3, KEYCODE_4};

  // 각 스위치 핀을 읽고 상태가 변했는지 확인한다
  for (int i = 0; i < 4; i++) {
    int reading = digitalRead(pins[i]);

    // 상태가 변했고 지연 시간이 지나면 새로운 상태로 업데이트
    if (reading != lastSwitchState[i] && currentMillis - lastDebounceTime[i] > DEBOUNCE_DELAY) {
      lastDebounceTime[i] = currentMillis;

      
      if (reading == LOW) {
        Keyboard.press(keycodes[i]); //눌리면 윈도우로 키 전송
        //특정 키 눌리면 진동 울림
        if (i == 0) {
          vibrationStartTime = currentMillis;
          vibrate_key = 1;
          isVibrating = true;

        }
        if (i == 1) {
          vibrationStartTime = currentMillis;
          vibrate_key = 2;
          isVibrating = true;

        }
      } 
      else {
        Keyboard.release(keycodes[i]);
      }

      // 스위치 상태를 갱신
      switchState[i] = reading;
    }

    // 현재 읽은 값을 이전 상태에 저장
    lastSwitchState[i] = reading;
  }



  // 진동 패턴 제어 로직
  if (isVibrating) {
    unsigned long elapsedTime = currentMillis - vibrationStartTime;
    if(vibrate_key == 1) {
      if (elapsedTime < 200) {
        vibratorOn = true;
      }
      else{
        isVibrating = false;
        vibratorOn = false;
        vibrate_key = -1;
      }
    } else if (vibrate_key == 2) {
      if (elapsedTime < 200) {
        vibratorOn = true;
      }
      else if (elapsedTime < 300) {
        vibratorOn = false;
      }
      else if (elapsedTime < 500) {
        vibratorOn = true;
      }
      else {
        isVibrating = false;
        vibratorOn = false;
        vibrate_key = -1;
      }
    } else{
      vibratorOn = false;
    }
    
  }

  // 진동자 동작 제어
  if (vibratorOn) {
    updateVibration();
  } else {
    analogWrite(VIBRATOR_PIN, 0);  // 진동자 신호 중단
  }
}

void updateVibration() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastVibrationUpdate >= 1) {  // 업데이트 주기
      int index;
      int signal;
      if (vibrate_key == 1) {
        // 둔탁한 진동 사용
        index = (int)((currentMillis - vibrationStartTime) * dullFrequency * TABLE_SIZE / 1000) % TABLE_SIZE;
        signal = pulseWaveTable[index];
      } else if (vibrate_key == 2) {
        // 날카로운 진동 사용
        index = (int)((currentMillis - vibrationStartTime) * sharpFrequency * TABLE_SIZE / 1000) % TABLE_SIZE;
        signal = pulseWaveTable[index];
      }
      analogWrite(VIBRATOR_PIN, signal);
      lastVibrationUpdate = currentMillis;
    }
}
