from tkinter import *
import time
from camera_test import *

def main():
    # creating tinker window
    root = Tk()
    gui = Window(root)
    root.wm_title("UI")
    root.geometry("320x320")
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
        camera_button.place(x=0, y=0)

    def click_exit_button(self):
        exit()

    def click_start_video(self):
        start_video()

if __name__ == '__main__':
    main()