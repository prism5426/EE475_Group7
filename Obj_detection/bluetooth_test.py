#! /user/bin/python

import serial
import time
import subprocess
import platform
import threading


print(platform.system())

def btConnect():
	return_code = subprocess.call("./SerialConnection.sh")
	print(return_code)
	
def comm():
	'''global connected
	while not connected:
		time.sleep(1)
		print("im waiting")
		continue'''
		
	bluetoothSerial = serial.Serial("/dev/rfcomm0", baudrate=9600, timeout=5)
	print("Bluetooth connected")
	print(bluetoothSerial.name)
	while 1:
		bluetoothSerial.write(str.encode('0123456789'));
		data = bluetoothSerial.read(10)
		print(data)
		time.sleep(1)
	
threading.Thread(target=btConnect).start()
timeout = 20
time.sleep(timeout)
threading.Thread(target=comm).start()




	
