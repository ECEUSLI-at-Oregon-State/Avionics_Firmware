import serial

# import gmplot package 
import gmplot
import codecs
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# Converts list to dictionary
def lst_to_dct(lst): 
    dct = {
        "Packet number": lst[0],
        "Altitude": lst[1],
        "Datetime": lst[2],
        "Latitude": lst[3],
        "Longitude": lst[4],
        "X-axis acceleration": lst[5],
        "Y-axis acceleration": lst[6],
        "Z-axis acceleration": lst[7]}
        # lst[i]: lst[i + 1] for i in range(0, len(lst) - 1, 2)} 
    return dct 

def animate(i):
    oldrow = []
    newrow = []

    logfile = open("Fullscale2-3-7-20-gui.csv", "a")                    # Open log file writer for appending
    # Decode and format string from Arduino
    line = ser.readline().decode('UTF-8', errors='ignore').strip('\n\r')
    if (len(line) > 0):
        logfile.write(line+'\n')
        newrow = line.split(',')                            # Convert comma-serparated string to list

        if newrow != oldrow:                                # If new packet
            data = lst_to_dct(newrow)
            print(data)                                     # Print as dictionary
        oldrow = newrow                                     # Current row list is now oldrow list

        x.append(i)
        if (len(x) > 50):
            x.pop(0)
        alt.append(data["Altitude"])
        x1.append(data["X-axis acceleration"])
        y1.append(data["Y-axis acceleration"])
        z1.append(data["Z-axis acceleration"])

        ax1.plot(x, alt)
        ax1.set_title('Altitude')
        ax2.plot(x, x1)
        ax2.set_title('X-axis acceleration')
        ax3.plot(x, y1)
        ax3.set_title('Y-axis acceleration')
        ax4.plot(x, z1)
        ax4.set_title('Z-axis acceleration')

        fig.canvas.draw();

        ax1.set_xlim(left=max(0, i-50), right=i+50)
        ax2.set_xlim(left=max(0, i-50), right=i+50)
        ax3.set_xlim(left=max(0, i-50), right=i+50)
        ax4.set_xlim(left=max(0, i-50), right=i+50)

        i += 1

data = {} 

# Script will run until CTRL-C is pressed
try:
    # Initialize serial port
    ser = serial.Serial()
    ser.baudrate = 9600
    ser.port = 'COM7'
    ser.open() 
    ser.timeout = 5                                         # 5 second timeout

    print("Script running (CTRL-C to exit)")

    fig = plt.figure()
    ax1 = fig.add_subplot(2,2,1)
    ax2 = fig.add_subplot(2,2,2)
    ax3 = fig.add_subplot(2,2,3)
    ax4 = fig.add_subplot(2,2,4)

    i = 0
    x, alt, x1, y1, z1 = [], [], [], [], []

    ani = animation.FuncAnimation(fig, animate, interval=1000)
    plt.show()
        
except KeyboardInterrupt:
    print("CTRL-C detected. Exiting...")
    # Close serial port
    ser.close()
    logfile.close()
    exit()