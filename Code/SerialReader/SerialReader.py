import serial
import serial.tools.list_ports
from serial.serialutil import SerialException
import time
import datetime as dt
import os
import numpy as np
import tkinter as tk
from tkinter import ttk
from tkinter import filedialog as fd
import sys

window = tk.Tk()
window.title("Lem")

status_label = tk.Label(text='Select the Firebeetle\'s port, it should be the same port as in the Arduino app, then select where to save the data.')
status_label.pack(ipadx=100, ipady=50)


def assign_port_values():
    ''' Re-builds the list of values for the combo box. '''
    port_selector["values"] = serial.tools.list_ports.comports()

#The post command is called every time the user opens the combo box.
port_selector = ttk.Combobox(window, textvariable=tk.StringVar(), postcommand=assign_port_values)
port_selector.pack(ipadx=100, pady=25)

assign_port_values() #First refresh.

port_selector.current(0)


def resource_path(relative_path):
    ''' Get absolute path to resource, works for dev and for PyInstaller '''
    try:
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS # Throws an exception when not run by PyInstaller
    except Exception:
        base_path = os.path.abspath("../../Logo/")

    return os.path.join(base_path, relative_path)


window.iconbitmap(resource_path("Logo.ico"))


def open_folder():
    ''' Asks the user where he wants to save the data, then transfers the files from the serial port. '''
    try:
        output_directory = fd.askdirectory()
        # Open a serial connection (modify 'COMX' to your Arduino's serial port)
        ser = serial.Serial(port_selector.get().split(' ')[0], 115200)

        # Create the output directory if it doesn't exist
        os.makedirs(output_directory, exist_ok=True)

        ser.write(b'R')

        while True:
            line = ser.readline().decode().strip()
            if line.startswith("Sending file: "):
                filename = line[14:]
                with open(output_directory + filename, 'wb') as f:
                    while True:
                        data = ser.readline()
                        newLine = data.decode().strip()
                        if newLine.endswith("End of file"):
                            break
                        f.write(data)

            elif line == "End":
                break

            else:
                time.sleep(1)  # Wait for more data

        ser.close()
        status_label.config(text=f"File download done at {dt.datetime.now().strftime("%Y/%m/%d %H:%M:%S")}")
    except SerialException as file_not_found:
        if str(file_not_found).index("PermissionError") >= 0:
            status_label.config(text=f"There was an error downloading files, you should close either the Serial Monitor or the Arduino app altogether.\nThe app couldn't read the data because someone is already connected to the device.\n This is the error generated : \n{file_not_found}")
        else:
            status_label.config(text=f"There was an error downloading files, you probably didn't select the right port. \n This is the error generated : \n{file_not_found}")
    except Exception as e:
        status_label.config(text=f"There was an error downloading files, this is the error generated : \n{e}")
    

button = tk.Button(text='Select Folder', command=open_folder)
button.pack(padx=250, pady=50)

window.mainloop()