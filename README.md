# Air-Quality-Monitoring-System
Arduino/NodeMCU programs that programatically calibrates the readings from a MQ2 &amp; MQ135 sensors that
detects smoke and other gases such as carbon monoxide, ammonia, methane in the air.

The sensors required for the sensing the gases is connected as an array to an arduino UNO board. 
An LCD display is  connected to the board to show the reading instantly on the device. In addition
the reading are stored as a json and communicated serially to a NodeMCU 

The Node MCU sets up wifi connection ,a Firebase connection, reads the json data from Arduino
and stores it in a Firebase database on the cloud. The timestamp required in the data was generated 
prgramatically by initially getting the date and time from any website at the time the device was
started and programatically updating it in a loop.

The programs were developed on sketch. The libraries provided are to be loaded in the libraries folder setup 
on your system. On windows platform usually located in the documents/arduino folder

Warning : 
The maximum value an unsigned long int(the variable used  as clock counter) can hold is 4,294,967,295. 
Convert that many milliseconds into days and the rollover will occur in 49 days. It would be safe to shut down the
system once a month for getting the correct date and time. The other option is to use RTC device connected to Node MCU


Setup:

Load the files under the libraries folder in the arduino sketch libraries path. 
Update the Declare.h file and provide the FIREBASE DB setup details and your WIFI credentials
