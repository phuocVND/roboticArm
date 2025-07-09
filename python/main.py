import cv2
import mediapipe as mp
import numpy as np
import socket

# Cấu hình UDP
UDP_IP = "192.168.1.100"  # IP của ESP32
UDP_PORT = 12345
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Hàm tính góc
def calculate_angle(a, b, c):
    a, b, c = np.array(a), np.array(b), np.array(c)
    ab = a - b
    cb = c - b
    cosine_angle = np.dot(ab, cb) / (np.linalg.norm(ab) * np.linalg.norm(cb))
    angle = np.arccos(np.clip(cosine_angle, -1.0, 1.0))
    return np.degrees(angle)

# Mediapipe pose
mp_pose = mp.solutions.pose
pose = mp_pose.Pose()

cap = cv2.VideoCapture(0)

while cap.isOpened():
    success, frame = cap.read()
    if not success:
        print("Không thể đọc khung hình từ camera.")
        break

    image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    image.flags.writeable = False
    results = pose.process(image)
    image.flags.writeable = True
    image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)

    if results.pose_landmarks:
        h, w, _ = image.shape
        lm = results.pose_landmarks.landmark

        angles = {}

        # Chỉ lấy các khớp bên phải
        angle_points = {
            "Right Shoulder": [24, 12, 14],
            "Right Elbow":    [12, 14, 16],
            "Right Wrist":    [14, 16, 22],
        }

        for label, (a_idx, b_idx, c_idx) in angle_points.items():
            try:
                a = [lm[a_idx].x * w, lm[a_idx].y * h]
                b = [lm[b_idx].x * w, lm[b_idx].y * h]
                c = [lm[c_idx].x * w, lm[c_idx].y * h]

                angle = calculate_angle(a, b, c)
                angle_uint8 = int(np.clip(angle, 0, 255))  # Giới hạn trong 0–255
                angles[label] = angle_uint8

                # Vẽ góc lên khung hình
                cv2.putText(image, f'{label}: {angle_uint8}',
                            (int(b[0]), int(b[1]) - 20),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
            except:
                pass

        # Gửi mảng uint8_t nếu đủ 3 góc
        if len(angles) == 3:
            data = bytes([
                angles['Right Shoulder'],
                angles['Right Elbow'],
                angles['Right Wrist']
            ])
            sock.sendto(data, (UDP_IP, UDP_PORT))

    cv2.imshow('Pose Angles', image)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
