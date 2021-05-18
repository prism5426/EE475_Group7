import time
import tkinter
#from camera_test import * TODO remove comment

UPDATE_RATE = 1000
 
class Application(tkinter.Frame):
    """ GUI """
 
    def __init__(self, master=None):
        """ Initialize the Frame"""

        self.counter = 0
        self.size = [[100,100,300,300],[100/2, 200/2, 400/2, 400/2]]
        super().__init__(master)
        self.master = master
        self.pack()
        self.create_button()
        self.create_canvas()
        #self.create_canvas()
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

    '''def create_radar(self, x1, y1, x2, y2):
        angle = 60
        self.arc = self.canvas.create_arc(x1, y1, x2, y2, start = 270-angle/2, extent = angle, outline = "green",\
                          fill = "blue", width = 2)'''

    def create_radar(self, x1, y1, x2, y2):
        angle = 60

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
        self.create_radar(100 + self.counter, 100, 300 - self.counter, 300 - self.counter)
        self.counter+=1
        '''if self.counter == 2:
            self.counter = 0'''
        
    def updater(self):
        #self.create_canvas()
        #self.update_canvas()
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
