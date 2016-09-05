# tiveda
Firmware for the TIVEDA esp8266 based GPS enabled device to notify drivers before fixed traffic control.

This software requires the Arduino IDE with the ESP8266 Arduino-Core installed

Requirements:

Arduino Event Manager
 * tested with commit 423e1dc3
 * https://github.com/igormiktor/arduino-EventManager

WiFiManager
 * patchad asynchron version, wich can be found here:
 * https://github.com/mrbig/WiFiManager
 * please use the f/non_blocking_api branch
