from picamera import PiCamera
from time import sleep
import cv2
cap = cv2.VideoCapture(0)

def start_video():

    while True:
        ret, frame = cap.read()
        cv2.imshow("cam", frame)
        if cv2.waitKey(1) & 0xFF == 27:
            break
        
    cap.release()
    cv2.destroyAllWindows()

def cap_video():
    ret, frame = cap.read()
    cv2image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    
    scale_percent = 80 # percent of original size
    width = int(cv2image.shape[1] * scale_percent / 100)
    height = int(cv2image.shape[0] * scale_percent / 100)
    dim = (width, height)

    cv2image = cv2.resize(cv2image, dim)
    return cv2image


#start_video()
#c1 = PiCamera()

#c1.start_preview()
#sleep(60)
#c1.stop_preview()


