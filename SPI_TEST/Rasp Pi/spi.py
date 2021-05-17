import spidev
import time
import RPi.GPIO as GPIO

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM) # Sets the GPIO pin labelling mode
spi = spidev.SpiDev()
SPI_SPEED = 10000
CS_PIN = 8

def setup():
    """Setup GPIO port for CS pin, and the SPI bus"""
    # setup CS
    
    GPIO.setup(CS_PIN, GPIO.OUT)
    GPIO.output(CS_PIN, 1)  # set CS initially to high. CS is pulled low to start a transfer
    # setup SPI
    spi.open(0, 0)  
    spi.max_speed_hz = SPI_SPEED
    spi.mode = 0b00

if __name__ == '__main__':
    setup()

data = {1,2,3,4,5,6,7,8}

try:
    while (1) :
        #assert CS
        GPIO.output(CS_PIN, 0)
        rcv = spi.xfer2(data)
        #release CS
        GPIO.output(CS_PIN, 1)
        print(f"rcv = {rcv}")
        time.sleep(5)
except KeyboardInterrupt:
    spi.close()