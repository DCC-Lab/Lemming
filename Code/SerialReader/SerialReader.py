import serial
import time
import datetime as dt
import os
import numpy as np
import tkinter as tk
from tkinter import ttk
from tkinter import filedialog as fd
import serial.tools.list_ports

window = tk.Tk()
window.title("Lemming")
window.iconbitmap("../../Logo/Logo.ico")

ports = serial.tools.list_ports.comports()

port_selector = ttk.Combobox(window, textvariable=tk.StringVar())
port_selector['values'] = ports
port_selector.pack(ipadx=100, pady=50)

port_selector.current(0)

status_label = tk.Label(text='Select the Arduino port then select where to save the data.')
status_label.pack(ipadx=100, ipady=50)

def open_folder():
    output_directory = fd.askdirectory()
    # Open a serial connection (modify 'COMX' to your Arduino's serial port)
    ser = serial.Serial(port_selector.get().split(' ')[0], 115200)

    # Directory to save received files
    # output_directory = "LEM_20231010/LEM_6"

    # Create the output directory if it doesn't exist
    os.makedirs(output_directory, exist_ok=True)

    # Initialize an empty NumPy array to accumulate data
    data_array = np.array([], dtype=np.uint8)

    # line = ser.readline().decode('utf-8').strip()
    # if line == "Start":
    #     start = True

    ser.write(b'R')

    while True:
        line = ser.readline().decode().strip()
        if line.startswith("Sending file: "):
            filename = line[14:]
            with open(output_directory + filename, 'wb') as f:
                while True:
                    data = ser.readline()
                    newLine = data.decode('utf-8').strip()
                    if newLine.endswith("End of file"):
                        break
                    f.write(data)

        elif line == "End":
            break

        else:
            time.sleep(1)  # Wait for more data

    ser.close()
    status_label.config(text=f"File download done at {dt.datetime.now().strftime("%Y/%m/%d %H:%M:%S")}")
    

button = tk.Button(text='Select Folder', command=open_folder)
button.pack(padx=250, pady=50)

window.mainloop()

# Now you have all the data in data_array as a NumPy array