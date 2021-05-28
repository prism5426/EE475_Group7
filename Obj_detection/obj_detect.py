import time
import tkinter
from PIL import Image, ImageTk
from i2c_test import * 
from camera_test import * 
import os
import threading
#from multiprocessing import Process
import sys
from functools import lru_cache

UPDATE_RATE = 50
 
class Application(tkinter.Frame):
    """ GUI """
    
    def __init__(self, master=None):
        """ Initialize the Frame"""
        self.THRESHOLD = 400
        self.counter = 0
        self.num_sensor = 4
        lr_offset = 180
        lr_h = 250
        button_offset = 50
        button_height = 320
        self.us_pos = [(lr_offset, lr_h, 'L'), (500/2-button_offset, button_height, 'D'), (500/2+button_offset, button_height, 'D'), (500-lr_offset, lr_h, 'R')]
        super().__init__(master)
        self.master = master
        self.pack()
        self.create_button()
        self.create_canvas()
        self.FIFO_LEN = 3
        self.dist_buffer = [[0 for i in range(self.FIFO_LEN)] for j in range(self.num_sensor)]
        self.dist_avg = [0 for i in range(self.num_sensor)]
        self.prev_arc = [[0 for i in range(self.num_sensor)] for j in range(self.num_sensor)]
        self.updater()


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

        self.canvas.pack(expand=False)

    def update_canvas(self):
        windX, windY = 500,500
        self.canvas.delete('all')
        self.canvas = tkinter.Canvas(self.master, bg='black', width = windX, height = windY)
        self.image = self.canvas.create_image(windX/2, windY/3, image=self.file)
        self.canvas.update()


    def create_radar(self):
        angle = 40
        h_arr = self.dist_avg
        print(h_arr)
        # print(sys.getsizeof(self.dist_avg), sys.getsizeof(self.dist_buffer))
        # use len(h_arr) for all sensors
        for i in range(self.num_sensor):
            cx, cy, direction = self.us_pos[i]
            x1, y1, x2, y2 = self.calc_coor(cx, cy, h_arr[i])
            
            # erase prev arc
            x1_old = self.prev_arc[i][0]
            y1_old = self.prev_arc[i][1]
            x2_old = self.prev_arc[i][2]
            y2_old = self.prev_arc[i][3]

            switcher={
            'L' : 180,
            'R' :0,
            'D' :270
            }
            start_angle = switcher.get(direction)
            #print(direction, start_angle)
            self.arc = self.canvas.create_arc(x1_old, y1_old, x2_old, y2_old, start = start_angle-angle/2, extent = angle, outline = "black",
                              fill = "black", width = 2)
            # update prev arc
            self.prev_arc[i][0] = x1
            self.prev_arc[i][1] = y1
            self.prev_arc[i][2] = x2
            self.prev_arc[i][3] = y2

            # draw new arc
            self.arc = self.canvas.create_arc(x1, y1, x2, y2, start=start_angle - angle / 2, extent=angle,
                                              outline="green",
                                              fill="blue", width=2)

        
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
        raw_data = read_data() 
        #print(self.distance)
        self.update_dist_buffer(cal_distance(raw_data))
        self.create_radar()
        self.after(UPDATE_RATE, self.updater)

    def update_dist_buffer(self, data):
        for i in range(self.num_sensor): 
            if (data[i] != -1) :
                prev = self.dist_buffer[i].pop()
                self.dist_buffer[i].insert(0, data[i])
                self.dist_avg[i] += (data[i] - prev) / self.FIFO_LEN

    def click_start_video(self):
        t1 = threading.Thread(target=os.system("python3 ml_test.py --modeldir coco_ssd_mobilenet_v1"), daemon=True)
        t1.start()

def app():
    root = tkinter.Tk()
    root.wm_title("UI")
    root.geometry("500x550")
    app = Application(master=root)
    app.mainloop()

if __name__ == "__main__":
    threading.Thread(target=app()).start()

