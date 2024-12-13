import cv2
import numpy as np
import mss
import serial
import time

arduino_port = "COM6"
baud_rate = 9600

# 아두이노 시리얼 연결 초기화
try:
    arduino = serial.Serial(arduino_port, baud_rate, timeout=1)
    print(f"아두이노 연결 성공: {arduino_port}")
except Exception as e:
    print(f"아두이노 연결 실패: {e}")
    arduino = None

# 캡처 영역 설정
regions = [
    {"top": 163, "left": 230, "width": 1200, "height": 2},
    {"top": 1395, "left": 700, "width": 1500, "height": 2},
    {"top": 1140, "left": 1165, "width": 45, "height": 45},
    {"top": 1450, "left": 1300, "width": 100, "height": 100},
]

# HSV 색상 범위
lower_red = np.array([0, 100, 100])
upper_red = np.array([10, 255, 150])
lower_yellow = np.array([20, 100, 100])
upper_yellow = np.array([30, 255, 180])

# 이전 상태 저장 변수
prev_hsv_frames = [None] * len(regions)
prev_color_distribution = None
same_color_start_time = None
prev_frame_region3 = None

with mss.mss() as sct:
    while True:
        for idx, region in enumerate(regions):
            # 화면 캡처
            screenshot = sct.grab(region)
            frame = np.array(screenshot)
            hsv_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

            if idx == 2:
                # 영역 2: 동일한 색상이 1.2초 이상 유지 확인
                current_color_distribution = cv2.calcHist(
                    [hsv_frame], [0, 1, 2], None, [8, 8, 8], [0, 180, 0, 256, 0, 256]
                ).flatten()

                if prev_color_distribution is not None:
                    similarity = cv2.compareHist(
                        prev_color_distribution, current_color_distribution, cv2.HISTCMP_CORREL
                    )

                    if similarity > 0.98:
                        if same_color_start_time is None:
                            same_color_start_time = time.time()
                        elif time.time() - same_color_start_time >= 1.2:
                            print(f"[영역 {idx}] 1.2초 동일 패턴 유지")
                            arduino_signal = '3'
                            if arduino:
                                try:
                                    arduino.write(arduino_signal.encode())
                                    print(f"상태 이상 신호 전송 완료: {arduino_signal}")
                                except Exception as e:
                                    print(f"상태 이상 신호 전송 실패: {e}")
                            same_color_start_time = None
                    else:
                        same_color_start_time = None
                else:
                    same_color_start_time = None

                prev_color_distribution = current_color_distribution

            elif idx == 3:
                # 영역 3: 평균 색상 계산, 급격한 변화 감지
                avg_color = np.mean(frame, axis=(0, 1))

                if prev_frame_region3 is not None:
                    frame_diff = cv2.absdiff(frame, prev_frame_region3)
                    diff_sum = np.sum(frame_diff)

                    if diff_sum > 20000:
                        # 디버깅 : print(f"[영역 {idx}] 평균 색상 색상 감지: {avg_color}")
                        if (115 < avg_color[0] < 145 and 115 < avg_color[1] < 145 and 115 < avg_color[2] < 145):
                            arduino_signal = 'W'  # 설원 지역 감지
                        elif 30 < avg_color[0] < 40 and 30 < avg_color[1] < 40 and 30 < avg_color[2] < 40 and not(avg_color[0] < avg_color[2]):
                            arduino_signal = 'B'  # 물가 지역 감지
                        else:
                            arduino_signal = None

                        if arduino_signal:
                            print(f"[영역 {idx}] 지역 색상 감지: {arduino_signal}")
                            if arduino:
                                try:
                                    arduino.write(arduino_signal.encode())
                                    print(f"지역 신호 전송 완료: {arduino_signal}")
                                except Exception as e:
                                    print(f"지역 신호 전송 실패: {e}")

                prev_frame_region3 = frame.copy()

            else:
                # 영역 0, 1: 체력 변화 감지 : 빨강 -> 노랑
                if prev_hsv_frames[idx] is not None:
                    mask_prev_red = cv2.inRange(prev_hsv_frames[idx], lower_red, upper_red)
                    mask_current_yellow = cv2.inRange(hsv_frame, lower_yellow, upper_yellow)
                    transition_mask = cv2.bitwise_and(mask_prev_red, mask_current_yellow)
                    transition_count = np.sum(transition_mask > 0)

                    if transition_count > 10:
                        print(f"[영역 {idx}] 변화 감지 : 빨강 -> 노랑")
                        arduino_signal = str(idx + 1)
                        if arduino:
                            try:
                                arduino.write(arduino_signal.encode())
                                print(f"체력 신호 전송 완료: {arduino_signal}")
                            except Exception as e:
                                print(f"체력 신호 전송 실패: {e}")

                prev_hsv_frames[idx] = hsv_frame.copy()

            cv2.imshow(f"Screen Capture - Region {idx}", frame)

        if cv2.waitKey(1) & 0xFF == 27:
            break

if arduino:
    arduino.close()
cv2.destroyAllWindows()
