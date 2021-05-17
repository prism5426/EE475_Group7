from tkinter import *
import time
from camera_test import *

def main(x1, y1, x2, y2):
    # creating tinker window
    root = Tk()

    gui = Window(root)
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

class Window(Frame):
    
    def __init__(self, master=None):

        
        Frame.__init__(self, master)
        self.master=master      
        
        self.pack(fill=BOTH, expand=1)
        menu = Menu(self.master)
        self.master.config(menu=menu)

        # Menu
        fileMenu = Menu(menu)
        #fileMenu.add_command(label="Item")
        fileMenu.add_command(label="Exit", command=self.click_exit_button)
        menu.add_cascade(label="Manu", menu=fileMenu)

        # button
        #exit_button = Button(self, text="Exit", command=self.click_exit_button)
        #exit_button.place(x=0, y=0)

        camera_button = Button(self, text="view camera feed", command=self.click_start_video)
        #camera_button.place(x=0, y=0)
        camera_button.pack()


    def click_exit_button(self):
        exit()

    def click_start_video(self):
        start_video()

if __name__ == '__main__':
    size = [[100, 200, 400, 400],[100/2, 200/2, 400/2, 400/2]]
    i = 0
    while True:
        if i == 2:
            i = 0 
        main(size[i][0], size[i][1], size[i][2], size[i][3])
        i += 1
        time.sleep(1)