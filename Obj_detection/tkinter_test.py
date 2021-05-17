from tkinter import *
 

 
def draw(root):
    frame=Frame(root,width=300,height=300)
    frame.pack(expand = True, fill=BOTH)
    
    canvas = Canvas(frame,bg='white', width = 300,height = 300)
    
    file = PhotoImage(file = "car_model.png")
    image = canvas.create_image(150, 150, image=file)
    
    canvas.pack(expand = True, fill = BOTH)
    return root
 
root = Tk()
draw(root)
root.mainloop()
