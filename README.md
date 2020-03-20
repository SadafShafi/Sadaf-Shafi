------------------------------------------------------ ![LOGO](https://github.com/SadafShafi/Sadaf-Shafi/blob/master/9ea05902-36d6-4a11-9c24-ab2b88fd4bd4_200x200.png)-------------------------------------------------

#GREEN DEVICE

This Device, as we decided, is called Green Device for :

1: Its prototype had a  Green colored body, and 

2: It was initially designed for Green houses.


Function:
    This device senses various parameters in a green house or some field then sends them to some server, from where this data is sent to some Device like a mobile phone in a real time, all the while this data is stored in a cloud, which could be used for analysis later on.
Specialities:
1.   This device takes care of the issues like when there is connection failure, it  pings the server several times and if it still fails to establish the connection  it saves the data in it database along with the time when it was supposed to be sent,so that on the user end it is clear how delay there has been.
2. WiFi Crediantials, Name of WiFi,API link,Sleep Time are not to be hard coded, rather can be changed by the user by connecting her device to it via direct WiFi, where Green device acts as an AP(access point).
3. The device goes to a sleep mode and wakes up after certain intervals, as desired by the user,  as a result saves the power consumption.


Hardware :


1. NodeMCU (esp8266) -- Micro-controller
2. DS18b20   -- Temperature sensor
3. BME 280 -- Ambient temperature,humidity,pressure(barometric)  sensor.
4. RTC -- real time clock
5. TSL2561 -- Luminosity sensor
6. Capacitive Soil Mositure sensor

CODE:
The code written in C++, in Arduino IDE
