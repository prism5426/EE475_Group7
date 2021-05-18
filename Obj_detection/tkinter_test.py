'''
from tkinter import *
import time
#from camera_test import *

def main(x1, y1, x2, y2):
    # creating tinker window
    root = Tk()

    gui = GUI(root)
    

    windX, windY = 500,500
    frame=Frame(root,width=windX,height=windY)
    frame.pack(expand = True, fill=BOTH)
    
    # car
    canvas = Canvas(frame,bg='black', width = windX, height = windY)
    
    file = PhotoImage(file = "car_model.png")
    image = canvas.create_image(windX//2, windY//3, image=file)
    


    #canvas.create_oval(60,60,210,210)

    canvas.create_arc(x1, y1, x2, y2, start = 210,
                          extent = 180-60, outline = "green",
                          fill = "blue", width = 2)
    
    canvas.pack(expand = False)
    root.wm_title("UI")
    #root.geometry("500x500")

    root.mainloop()

class GUI(Frame):
    
    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.master=master      
        #self.create_manu()
        self.create_button()
            
        #self.pack(fill=BOTH, expand=1)
        #menu = Menu(self.master)
        #self.master.config(menu=menu)

    def create_manu(self):
        # Menu
        self.fileMenu = Menu(self)
        #fileMenu.add_command(label="Item")
        #self.fileMenu.add_command(label="Exit", command=self.click_exit_button)
        #menu.add_cascade(label="Manu", menu=fileMenu)

    def create_button(self):
        # button
        #exit_button = Button(self, text="Exit", command=self.click_exit_button)
        #exit_button.place(x=0, y=0)

        self.camera_button = Button(self, text="view camera feed", command=self.click_start_video)
        #camera_button.place(x=0, y=0)
        self.camera_button.pack()


    def click_exit_button(self):
        exit()

    def click_start_video(self):
        return None
        #start_video()

if __name__ == '__main__':
    
    size = [[100, 200, 400, 400],[100/2, 200/2, 400/2, 400/2]]
    i = 0
    while True:
        if i == 2:
            i = 0 
        main(size[i][0], size[i][1], size[i][2], size[i][3])
        i += 1
        time.sleep(1)

    
    main(100, 200, 400, 400)
'''

from tkinter import *
 
root = Tk()
 
frame=Frame(root,width=300,height=300)
frame.pack(expand = True, fill=BOTH)
 
canvas = Canvas(frame,bg='white', width = 300,height = 300)
 
file = PhotoImage(file = "download.png")
image = canvas.create_image(150, 150, image=file)
 
canvas.pack(expand = True, fill = BOTH)
 
root.mainloop()


