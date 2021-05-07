from picamera import PiCamera
from time import sleep
import cv2

def start_video():
    cap = cv2.VideoCapture(0)

    while True:
        ret, frame = cap.read()
        cv2.imshow("cam", frame)
        if cv2.waitKey(1) & 0xFF == 27:
            break
        
    cap.release()
    cv2.destroyAllWindows()

#start_video()
#c1 = PiCamera()

#c1.start_preview()
#sleep(60)
#c1.stop_preview()


