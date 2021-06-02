#!/usr/bin/env python
# coding: utf-8

import RPi.GPIO as GPIO
import time



def Distance(): #get the distance between obstacle and car
    GPIO.output(23, True) #set trigger to high
    time.sleep(0.00001)
    GPIO.output(23, False) #set trigger to low
    
    #assign start and stop at current time
    start = time.time()
    stop = time.time()
    
    #when echo is high, refresh start
    while GPIO.input(22) == 0:
        start = time.time()
    #when echo from high to low, refresh stop 
    while GPIO.input(22) == 1:
        stop = time.time()
    #get the time that the echo goes back
    Measure = stop - start
    #calculate the distance 
    dist = Measure * 33112
    distance = dist / 2
    
    return distance


def beep(): #function that define beep frequency
    measureDist = Distance()
    
    #the value below are 1m, 0.5m, 0.3m, and 0.1m for a real car
    #for testing, we can use 50cm, 30cm, 20cm, and 10cm
    #when distance larger than 1m, no beeping:
    if measureDist > 100:
        return -1
    #when distance larger than 0.5m, beep once a second
    elif measureDist <= 100 and MeasureDist >= 50:
        return 1
    #when distance larger than 0.3m, beep twice a second
    elif measureDist < 50 and MeasureDist >= 30:
        return 0.5
    #when distance larger than 0.1m, beep 4 times a second
    elif measureDist < 30 and MeasureDist >= 10:
        return 0.25
    #otherwise, no beeping
    else: 
        return 0



def main():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(18, GPIO.OUT)
    pwm = GPIO.PWM(18, 2000)
    #pwm.ChangeDutyCycle(50)
    pwm.start(0)
    #while 1:
    #    None
    
    while True:
        for dc in range(10, 101, 1):
            pwm.ChangeDutyCycle(dc)
            time.sleep(0.01)
        time.sleep(1)
        
        for dc in range(100, 10, -1):
            pwm.ChangeDutyCycle(dc)
            time.sleep(0.01)
        time.sleep(1)
    
    '''
    try:
        #repeat until the system end
        while True:
            #get frequency
            #freq = beep()
            freq = 0.25
            if freq == -1: #no beeping
                GPIO.output(18, False)
                time.sleep(0.25)
            elif freq == 0: #constant beep for distance less than 0.1m
                GPIO.output(18, True)
                time.sleep(0.25)
            else: #beep on certain frequency
                GPIO.output(18, True)
                time.sleep(0.2)
                GPIO.output(18, False)
                time.sleep(freq)
    except KeyboardInterrupt:
        GPIO.output(18, False)
        GPIO.cleanup()
    '''
        
if __name__ == "__main__":
    main()
                

