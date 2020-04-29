import matplotlib.pyplot as plt
import numpy as np
import serial
import time

Fs=400


t = np.arange(0,10,0.1)
#t2 = np.arange(0,10,0.1)
x = np.arange(0,10,0.1)
y = np.arange(0,10,0.1)
z = np.arange(0,10,0.1)
ts = np.arange(0,10,0.1)
serdev = '/dev/ttyACM0'
s = serial.Serial(serdev,115200)
xi=0
yi=0
zi=0
ti=0
for i in range(0, int(Fs)):
    line = s.readline().decode()
    if 'x' in line:
        x[xi]=line[1:]
        xi=xi+1
    elif 'y' in line:
        y[yi]=line[1:]
        yi=yi+1
    elif 'z' in line:
        z[zi]=line[1:]
        zi=zi+1
    elif 't' in line:
        ts[ti]=line[1:]
        ti=ti+1
        




fig, ax = plt.subplots(2, 1)
ax[0].plot(t,x,color="blue", linewidth=1.0, linestyle="-",label="x")
ax[0].plot(t,y,color="red", linewidth=1.0, linestyle="-",label="y")
ax[0].plot(t,z,color="green", linewidth=1.0, linestyle="-",label="z")
ax[0].legend( loc='lower left', borderaxespad=0.)
ax[0].set_xlabel('Time')
ax[0].set_ylabel('Acc Vector')
ax[1].stem(t,ts) # plotting the spectrum

ax[1].set_xlabel('Time')
ax[1].set_ylabel('more than5')
plt.show()
s.close()   