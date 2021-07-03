from pynput.keyboard import Key, Controller
import time
import tkinter as Tkinter
from tkinter import *
from datetime import datetime, timedelta
import serial.tools.list_ports
import serial
import threading, sys, os
from csv import writer
import sys
import qrcode
from qrcode.image.pure import PymagingImage
import png
from PIL import ImageTk,Image  

counter = 10
global delayedCounter
running = False

ipAdressIsShown = False

###### Variables
welcome_message = "ESPDeck\nStarte um ESPDeck zu verwenden"


##
##
##
##  This is only ESPDeck
##
##
##


keyboard = Controller()
def hit_shortcut(key):
    keyboard.press(Key.ctrl)
    keyboard.press(Key.shift)
    keyboard.press(key)
    time.sleep(0.3)
    keyboard.release(key)
    keyboard.release(Key.shift)
    keyboard.release(Key.ctrl)


# Shortcuts are always Strg+Shift+[key]
# hit_shortcut("a") -> Strg+Shift+A#

"""

while True:
    serialString = str(pairer.readline())
    serialString = serialString[2:][:-5]
            
    if (serialString) and "x0elrlp" not in serialString:
        print(serialString)
        x = serialString.split('+')
        if len(x) >= 2:
            hotkey = x[2][0]
            hit_shortcut(hotkey)

"""
##
##
##
##  END ONLY ESPDEck
##
##
##

def get_sec(time_str):
    """Get Seconds from time."""
    h, m, s = time_str.split(':')
    return int(h) * 3600 + int(m) * 60 + int(s)

def get_current_timestamp():
    return  int(time.mktime(datetime.today().timetuple()))    

def getTimeFromSeconds(secondsInput):
    tt = datetime.fromtimestamp(secondsInput) - timedelta(hours = 1)
    string = tt.strftime("%H:%M:%S")
    return string

startStamp = 0
currentStamp = 0 
startStamp = get_current_timestamp()
lastStamp = get_current_timestamp()
duration = 0


def counter_label(label):
    def count():
        if running:
            global counter
            global currentStamp
            global lastStamp
            global duration
            currentStamp = get_current_timestamp()
            if currentStamp - lastStamp >= 1:
                lastStamp = get_current_timestamp()
                counter += 1
                runningTime = getTimeFromSeconds(counter)
                label['text']=runningTime
                
            time.sleep(0.2)
   
    # Triggering the start of the counter.
    count()     
   
# start function of the stopwatch
def Start(label):
    global counter
    delayedCounter = get_sec(TextBoxCounterDelay.get("1.0", Tkinter.END))
    counter = delayedCounter - 1
    #print(counter)

    global running
    running=True
    counter_label(label)

    start['state']='disabled'
    stop['state']='normal'
    reset['state']='disabled'
   
# Stop function of the stopwatch
def Stop():
    global running
    global display
    start['state']='normal'
    stop['state']='disabled'
    reset['state']='normal'
    running = False

    #print(label['text'])

    stoppedTime = get_sec(label['text'])
    stoppedTime = stoppedTime + 5

    tt = datetime.fromtimestamp(stoppedTime) - timedelta(hours = 1)
    string = tt.strftime("%H:%M:%S")

    TextBoxCounterDelay.delete("1.0","end")
    TextBoxCounterDelay.insert(1.0, string, "center")
   
# Reset function of the stopwatch
def Reset(label):
    global counter
    global startStamp
    global currentStamp
    startStamp = 0
    currentStamp = 0 
    counter=30
   
    # If rest is pressed after pressing stop.
    if running==False:   
        reset['state']='disabled'
        label['text']= welcome_message
        tt = datetime.fromtimestamp(counter) - timedelta(hours = 1)
        string = tt.strftime("%H:%M:%S")
        TextBoxCounterDelay.delete("1.0","end")
        TextBoxCounterDelay.insert(1.0, string, "center")
   
    # If reset is pressed while the stopwatch is running.
    else:               
        label['text']='Starte ...'
   
root = Tkinter.Tk()
root.title("ESPDeck")
   
# Fixing the window size.
root.minsize(250, 115)
root.maxsize(250, 115)

label = Tkinter.Label(root, text=welcome_message, fg="black", font="Verdana 8 bold")
label.pack()
f = Tkinter.Frame(root)


start = Tkinter.Button(f, text='Start', width=6, command=lambda:Start(label))
stop = Tkinter.Button(f, text='Stop',width=6,state='disabled', command=Stop)
reset = Tkinter.Button(f, text='Reset',width=6, state='disabled', command=lambda:Reset(label))

TextBoxCounterDelay = Tkinter.Text(root, height=1, width=15)
TextBoxCounterDelay.pack()
TextBoxCounterDelay.tag_configure("center", justify='center')
TextBoxCounterDelay.delete("1.0","end")
TextBoxCounterDelay.insert(1.0, "00:00:10", "center")


f.pack(anchor = 'center',pady=5)
start.pack(side="left")
stop.pack(side ="left")
reset.pack(side="left")


markTakenLabel = Tkinter.Label(root, text="Marke wurde erstellt", font='Vrdana 9 bold', fg='#F00', padx = 10, pady = 5)
markTakenLabel.pack()
markTakenLabel.pack_forget()


link1 = Tkinter.Label(root, text="IP-ADRESSE", fg="blue", cursor="hand2")
link1.pack()
link1.pack_forget()


is_paired = False
com_port = ""


def toggleLabel(Label):
    label['fg']="#f00"
    Label.pack()
    time.sleep(3)
    Label.pack_forget()
    label['fg']="#000"

def on_closing():
    os._exit(1)



## Debug: List all Ports
#print([port.name for port in all_serial_ports])
messageDisplayed = False

while not is_paired: 
    all_serial_ports = serial.tools.list_ports.comports()
    for port in all_serial_ports:
        try:
            pairer = serial.Serial(port=port.name, baudrate=115200, timeout=.1)
            serialString = str(pairer.readline())
            serialString = serialString[2:][:-5]
            
            if messageDisplayed == False: 
                messageDisplayed = True
                print("Suche nach ESPDeck läuft ...")

            if serialString:
                if serialString == "pair:StreaMark":
                    print("StreaMark gefunden!")
                    is_paired = True
                    com_port = port.name
                    pairer.write(b'pair:StreaMark')
                    pairer.close()
        except:
            print ("Nächster Versuch...")
            time.sleep(5)

## Debug
## print("Der gefundene Port ist: " + com_port)

esp8266 = serial.Serial(port=com_port, baudrate=115200, timeout=.1)



filename = "markers.csv" 
# green, red, rose, orange, yellow, white, blue, teal
color = "red"
# chapter, segmentation,weblink,comment
markerType = "segmentation"

def create_marker(counter):
    tt = datetime.fromtimestamp(counter) - timedelta(hours = 1)
    string = tt.strftime("%H:%M:%S")
    print("Neuen Marker @ " + string)
    toggleLabel(markTakenLabel)
    markerData = [string +"\t"+ color +"\t"+ markerType]
    with open(filename, 'a') as f_object:
        writer_object = writer(f_object)
        writer_object.writerow(markerData)
        f_object.close()


#######################################
#### Serial Read


## Pairing is done, check for input messages
def read_from_port(serial_port):
    while True:
        serialString = str(esp8266.readline())
        serialString = serialString[2:][:-5]
        if (serialString) and "x0elrlp" not in serialString:

            if "add:marker" in serialString:
                create_marker(counter)           
            
            #print(serialString)
            x = serialString.split('+')
            
            if len(x) >= 2:
                hotkey = x[2][0]
                hit_shortcut(hotkey)

            if "ip-adress:1" in serialString:
                global ipAdressIsShown
                c = serialString.split(":")
                ipadress = c[1]
                #print("Die IP-Adresse lautet: " + ipadress)
                if not ipAdressIsShown:
                    ipAdressIsShown = True
                    
                    
                    # Generate QR Code
                    #Creating an instance of qrcode
                    qr = qrcode.QRCode(
                            version=1,
                            box_size=4,
                            border=1)
                    qr.add_data("http://" + ipadress + "/")
                    qr.make(fit=True)
                    img = qr.make_image(fill='black', back_color='white')
                    img.save('qrcode.png')
                    

                    
                    img = ImageTk.PhotoImage(Image.open("qrcode.png"))  
                    canvasQRCode.create_image(70, 5, anchor=NW, image=img) 

                    ipAdressIsShown = True
                    link1 = Tkinter.Label(canvasIPLabel, text=ipadress, fg="black")
                    link1.pack()

                    # make window bigger
                    root.minsize(250, 230)
                    root.maxsize(250, 230)
                    
                   

                    
                   
                    

thread_serial_read = threading.Thread(target=read_from_port, args=(esp8266,))
try:
    thread_serial_read.start()
except (KeyboardInterrupt, SystemExit):
    os._exit(1)


def lets_count():
    while True:
        counter_label(label)

thread_update_label = threading.Thread(target=lets_count)

try:
    thread_update_label.start()
except (KeyboardInterrupt, SystemExit):
    os._exit(1)




photo = PhotoImage(file = "assets/img/mark.png")
root.wm_iconphoto(False, photo)

canvasIPLabel = Tkinter.Canvas(root, width = 250, height = 10)  
canvasIPLabel.pack()  


canvasQRCode = Tkinter.Canvas(root, width = 250, height = 250)  
canvasQRCode.pack()  


root.protocol("WM_DELETE_WINDOW", on_closing)
root.mainloop()