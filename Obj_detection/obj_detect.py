import time
import tkinter
from i2c_test import *
#from camera_test import * TODO remove comment

UPDATE_RATE = 100
 
class Application(tkinter.Frame):
    """ GUI """
 
    def __init__(self, master=None):
        """ Initialize the Frame"""
        self.THRESHOLD = 300
        self.counter = 0
        self.size = [[100,100,300,300],[100/2, 200/2, 400/2, 400/2]]
        super().__init__(master)
        self.master = master
        self.pack()
        self.create_button()
        self.create_canvas()
        #self.create_canvas()
        self.distance = []
        self.prev_arc = [0, 0, 0, 0]
        self.updater()


 
    def create_button(self):
        self.button1 = tkinter.Button(self, text="view camera feed", command=self.click_start_video)
        self.button1.pack()

    def create_canvas(self):
        windX, windY = 500,500
        self.canvas = tkinter.Canvas(self.master, bg='black', width = windX, height = windY)
        self.file = tkinter.PhotoImage(file = "car_model.png")
        self.image = self.canvas.create_image(windX//2, windY//3, image=self.file)

        self.canvas.pack(expand=False)

    def update_canvas(self):
        windX, windY = 500,500
        self.canvas.delete('all')
        self.canvas = tkinter.Canvas(self.master, bg='black', width = windX, height = windY)
        self.image = self.canvas.create_image(windX//2, windY//3, image=self.file)
        self.canvas.update()


    def create_radar(self, x1, y1, x2, y2):
        angle = 40

        # erase prev arc
        x1_old = self.prev_arc[0]
        y1_old = self.prev_arc[1]
        x2_old = self.prev_arc[2]
        y2_old = self.prev_arc[3]
        self.arc = self.canvas.create_arc(x1_old, y1_old, x2_old, y2_old, start = 270-angle/2, extent = angle, outline = "black",
                          fill = "black", width = 2)
        # update prev arc
        self.prev_arc[0] = x1
        self.prev_arc[1] = y1
        self.prev_arc[2] = x2
        self.prev_arc[3] = y2

        # draw new arc
        self.arc = self.canvas.create_arc(x1, y1, x2, y2, start=270 - angle / 2, extent=angle,
                                          outline="green",
                                          fill="blue", width=2)

    def update_radar(self):
        print(self.counter)
        i = self.counter
        # self.create_radar(self.size[i][0], self.size[i][1], self.size[i][2], self.size[i][3])
        # self.create_radar(100 - self.counter, 100, 300 + self.counter, 300 + self.counter)
        
        x1, y1, x2, y2 = self.calc_coor(200, 320, self.distance[0], self.distance[0])
        #self.canvas.create_rectangle(x1, y1, x2, y2, outline = "red")
        self.create_radar(x1, y1, x2, y2)
        self.counter+=10
        '''if self.counter == 2:
            self.counter = 0'''
        
    def calc_coor(self, cx, cy, w, h):
        if w < 0 or h < 0:
            w = 0
            h = 0
        elif h > self.THRESHOLD:
            h = self.THRESHOLD
            w = self.THRESHOLD
        #x1, y1, x2, y2 = 0,0,0,0
    
        y1 = cy - h/2
        y2 = cy + h/2
        x1 = cx - w/2
        x2 = cx + w/2
        return x1, y1, x2, y2

    def updater(self):
        #self.create_canvas()
        #self.update_canvas()
        raw_data = read_data()
        self.distance = cal_distance(raw_data)
        print(self.distance)
        self.update_radar()
        self.after(UPDATE_RATE, self.updater)
        

    def click_start_video(self):
        return None
        #start_video() TODO remove

 
root = tkinter.Tk()
root.wm_title("UI")
root.geometry("500x500")
app = Application(master=root)
app.mainloop()
