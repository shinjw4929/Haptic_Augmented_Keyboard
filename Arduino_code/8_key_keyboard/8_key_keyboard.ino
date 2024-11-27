#include <Keyboard.h>

//스위치 & 핀
#define SWITCH_1 2
#define SWITCH_2 3
#define SWITCH_3 4
#define SWITCH_4 5
#define SWITCH_5 6
#define SWITCH_6 7
#define SWITCH_7 8
#define SWITCH_8 9

//키보드 관련
int pins[8] = {SWITCH_1, SWITCH_2, SWITCH_3, SWITCH_4, SWITCH_5, SWITCH_6, SWITCH_7, SWITCH_8};
char keycodes[8] = {'q', 'w', 'e', 'r', 'a', 's', 'd', 'f'};

bool currentSwitchState[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};  // 눌리기 전
bool prevSwitchState[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};  // 이전 상태

#define DEBOUNCE_DELAY 20 //채터링 방지 시간(millisecond)
unsigned long lastDebounceTime[8] = {0, 0, 0, 0, 0, 0, 0, 0};


void setup() {
  pinMode(SWITCH_1, INPUT_PULLUP);
  pinMode(SWITCH_2, INPUT_PULLUP);
  pinMode(SWITCH_3, INPUT_PULLUP);
  pinMode(SWITCH_4, INPUT_PULLUP);
  pinMode(SWITCH_5, INPUT_PULLUP);
  pinMode(SWITCH_6, INPUT_PULLUP);
  pinMode(SWITCH_7, INPUT_PULLUP);
  pinMode(SWITCH_8, INPUT_PULLUP);
}

void loop() {
  unsigned long currentMillis = millis();
  
  for (int i = 0; i < 8; i++) {
    currentSwitchState[i] = digitalRead(pins[i]);
  }

  for (int i = 0; i < 8; i++) {
    if (currentSwitchState[i] != prevSwitchState[i] && currentMillis - lastDebounceTime[i] > DEBOUNCE_DELAY) {
    
      lastDebounceTime[i] = currentMillis;

      if (currentSwitchState[i] == LOW) {

        Keyboard.press(keycodes[i]);
      }
      else {

        Keyboard.release(keycodes[i]);
      }
    }
  
    prevSwitchState[i] = currentSwitchState[i];
  }
}
