import time
import tkinter
from tkinter import messagebox
from PIL import Image, ImageTk
from i2c_test import * 
from camera_test import * 
from ml_test import *
import os
import threading
import sys
from functools import lru_cache
from beep_system import *

UPDATE_RATE = 50
SENSORS = [0, 1, 4, 5]
 
class Application(tkinter.Frame):
    """ GUI """
    
    def __init__(self, master=None):
        ''' Init ML predictor '''
        self.interpreter = app_args()

        ''' Init audio system'''
        self.pwm = audio_config()

        """ Initialize the Frame"""
        self.THRESHOLD = 400
        self.counter = 0
        self.num_sensor = 4
        self.update_counter = 0
        lr_offset = 180
        lr_h = 250
        button_offset = 50
        button_height = 320
        self.us_pos = [(lr_offset, lr_h, 'L'), (500/2-button_offset, button_height, 'D'), (500/2+button_offset, button_height, 'D'), (500-lr_offset, lr_h, 'R')]
        super().__init__(master)
        self.master = master
        self.pack()
        self.master.protocol("WM_DELETE_WINDOW", self.on_closing)
        # self.create_button()
        self.create_canvas()
        self.create_camera()
        self.FIFO_LEN = 3
        self.dist_buffer = [[0 for i in range(self.FIFO_LEN)] for j in range(self.num_sensor)]
        self.dist_avg = [0 for i in range(self.num_sensor)]
        self.arcs = [None for i in range(self.num_sensor)]
        self.updater()

    def on_closing(self):
        if messagebox.askokcancel("Quit", "Do you want to quit?"):
            GPIO.setup(18, GPIO.OUT)
            GPIO.output(18, False)
            GPIO.cleanup()
            self.master.destroy()

    def create_camera(self):
        windX, windY = 500,500
        self.video = tkinter.Label(bg='black', width = windX, height = windY)
        self.video.image = None
        self.video.pack(expand=False, side="left")

    def update_feed(self):
        frame = cap_video()
        self.update_counter += 1
        if self.update_counter == 10:
            update_prediction = True
            self.update_counter = 0
        else:
            update_prediction = False

        frame = predict(self.interpreter, frame, update_prediction=update_prediction)
        image = Image.fromarray(frame)
        image = ImageTk.PhotoImage(image)
        self.video.configure(image=image)
        self.video.image=image


    def create_button(self):
        self.button1 = tkinter.Button(self, text="view camera feed", command=self.click_start_video)
        self.button1.pack()

    def create_canvas(self):
        windX, windY = 500,500
        self.canvas = tkinter.Canvas(self.master, bg='black', width = windX, height = windY)
        self.img = Image.open("car_model.png")
        self.img = self.img.resize((500, 340))
        self.img = ImageTk.PhotoImage(self.img)
        self.image = self.canvas.create_image(windX/2+5, windY/3, image=self.img)

        self.canvas.pack(expand=False, side="left")
       
    # depricated
    def update_canvas(self):
        windX, windY = 500,500
        self.canvas.delete('all')
        self.canvas = tkinter.Canvas(self.master, bg='black', width = windX, height = windY)
        self.image = self.canvas.create_image(windX/2, windY/3, image=self.file)
        self.canvas.update()


    def create_radar(self):
        angle = 40
        print(self.dist_avg)
        # print(sys.getsizeof(self.dist_avg), sys.getsizeof(self.dist_buffer))
        # use len(self.dist_avg) for all sensors
        for i in range(self.num_sensor):
            cx, cy, direction = self.us_pos[i]
            x1, y1, x2, y2 = self.calc_coor(cx, cy, self.dist_avg[i])
            
            # erase prev arc if it exists
            if self.arcs[i] is not None:
                self.canvas.delete(self.arcs[i])

            switcher={
            'L' : 180,
            'R' : 0,
            'D' : 270
            }
            start_angle = switcher.get(direction)
            #print(direction, start_angle)
            self.arcs[i] = self.canvas.create_arc(x1, y1, x2, y2, start=start_angle - angle / 2, extent=angle, outline="green", fill="blue", width=2)
            
        
    def calc_coor(self, cx, cy, h):
        if  h < 0:
            h = 0
        elif h > self.THRESHOLD:
            h = self.THRESHOLD
    
        y1 = cy - h/2
        y2 = cy + h/2
        x1 = cx - h/2
        x2 = cx + h/2
        return x1, y1, x2, y2

    def updater(self):
        dist_data = cal_distance(read_data())
        # print(dist_data)
        # print(self.distance)
        self.update_dist_buffer(dist_data)
        self.create_radar()
        self.update_feed()

        min_dist_all_senesor = self.min_dist_non_zero(self.dist_avg)
        beeping_freq = beep(min_dist_all_senesor, real_world=False)
        update_audio(self.pwm, beeping_freq) # change to True if actually real-world

        self.after(UPDATE_RATE, self.updater)

    def update_dist_buffer(self, data):
        for i in range(self.num_sensor):
            j = SENSORS[i]
            if (data[j] != -1):
                prev = self.dist_buffer[i].pop()
                self.dist_buffer[i].insert(0, data[j])
                self.dist_avg[i] += (data[j] - prev) / self.FIFO_LEN

    # depricated
    def click_start_video(self):
        t1 = threading.Thread(target=os.system("python3 ml_test.py --modeldir coco_ssd_mobilenet_v1"), daemon=True)
        t1.start()

    def min_dist_non_zero(self, list):
        res = -1
        for i in list:
            if i > 0 and i > res:
                res = i
        return res

def app():
    try:
        root = tkinter.Tk()
        root.wm_title("Parking Assistant")
        root.geometry("1000x500")        
        app = Application(master=root)

        app.mainloop()
    except KeyboardInterrupt:
        #GPIO.output(18, False)
        #GPIO.cleanup()
        print("terminating")

if __name__ == "__main__":
    threading.Thread(target=app()).start()

