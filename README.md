# esp8266connect2wifi
ESP8266 connecting to existing WiFi or opening an AccessPoint to fill in SSID and PW

The .h file contains a function called connect2wifiorstartAccesPoint(), which reads a saved WiFi SSID and Password from a text file created in the SPIFFS file system
on the ESP8266. It tries to connect to the WiFi, if that failes, it creates an AccessPoint.

You can connect to the "AccessPointName" which can be named differently in the .h file. 
After that, open a browser on the connected device and type in the searchbar: 192.168.4.1
That will bring you to the webserver of the ESP and you can enter an SSID and password, which will be stored in the file system of the ESP.

The ESP will give feedback about the connecting process over the Serial terminal with a baudrate of 9600.

