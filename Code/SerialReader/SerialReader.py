import serial
import time
import os
import numpy as np
import tkinter as tk
from tkinter import filedialog as fd

window = tk.Tk()

def open_folder():
    output_directory = fd.askdirectory()
    # Open a serial connection (modify 'COMX' to your Arduino's serial port)
    ser = serial.Serial('/dev/cu.usbserial-110', 9600)

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
        line = ser.readline().decode('utf-8').strip()
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
    

button = tk.Button(text='Refresh', command=open_folder)
button.pack(padx=50, pady=50)

window.mainloop()

# Now you have all the data in data_array as a NumPy array