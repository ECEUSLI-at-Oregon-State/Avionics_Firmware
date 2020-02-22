import sys
import tkinter as tk
import pandas as pd
import csv
import time
from pandas import DataFrame
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from map import display

root = tk.Tk()


datetime = []
altitude = []
lat = []
lon = []
fig = plt.figure(figsize=(5,5))
ax = fig.add_subplot(111)
graph = FigureCanvasTkAgg(fig,root)
# graph.get_tk_widget().pack()
ax.plot(datetime,altitude,'b')
plt.title('Date Time vs Altitude')
plt.xlabel('Date Time')
plt.ylabel('Altitude')
plt.legend(['Altitude'])
plt.xticks(rotation=45, ha='right')




xText = tk.Label(root, text=1, font=('', 20))
xText.pack()
xText['text'] = "X-Acceleration"


xLabel = tk.Label(root, text=1, font=('', 20))
xLabel.pack()


yText = tk.Label(root, text=1, font=('', 20))
yText.pack()
yText['text'] = "Y-Acceleration"

yLabel = tk.Label(root, text=1, font=('', 20))
yLabel.pack()

zText = tk.Label(root, text=1, font=('', 20))
zText.pack()
zText['text'] = "Z-Acceleration"

zLabel = tk.Label(root, text=1, font=('', 20))
zLabel.pack()



def x(num):
    with open('test.csv') as csv_file:
        csv_reader = csv.reader(csv_file)
        rows = list(csv_reader)
        data = rows[num]
        print("X-Acceleration: ", data[5])
        return data[5]

def y(num):
    with open('test.csv') as csv_file:
        csv_reader = csv.reader(csv_file)
        rows = list(csv_reader)

        data = rows[num]
        print("Y-Acceleration: ", data[6])
        return data[6]

def z(num):
    with open('test.csv') as csv_file:
        csv_reader = csv.reader(csv_file)
        rows = list(csv_reader)
        data = rows[num]
        print("Z-Acceleration: ", data[7])
        return data[7]



def printSomething(count = 0):
    if count is 0:
        xLabel['text'] = 0
        yLabel['text'] = 0
        zLabel['text'] = 0
        test(0)
    else:
        xLabel['text'] = x(count)
        yLabel['text'] = y(count)
        zLabel['text'] = z(count)
        test(count)
        print("-------------------------------------------------")
    root.after(500, printSomething, count+1)





def getInfo(num):
    with open('test.csv') as csv_file:
        csv_reader = csv.reader(csv_file)
        rows = list(csv_reader)
        data = rows[num]
        return data


def test(num):
    global datetime
    global altitude
    global lat
    global lon

    info = getInfo(num)
    if(num is 0):
        ax.plot(datetime,altitude,'b')
    else:
        datetime.append(info[2])
        altitude.append(info[1])
        lat.append(info[3])
        lon.append(info[4])
        ax.plot(datetime,altitude,'b')


    fig.canvas.draw()
    fig.canvas.flush_events()

    graph.get_tk_widget().pack()
    #root.after(600, test,num+1)




printSomething()
root.mainloop()
display(lat,lon)
