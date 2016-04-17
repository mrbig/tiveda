
#include "config.h"
#include <EventManager.h>

#include "gps.h"

#ifdef ENABLE_STATUSLED
#include "statusled.h"
#endif

EventManager eventManager;

// Buffer to collect incoming data on serial
String serialBuffer = "";

// Holds the last complete line received
String message;



void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.setDebugOutput(true);
    serialBuffer.reserve(200);
    message.reserve(200);

    GPS::init(&eventManager, &message);

#ifdef ENABLE_STATUSLED
    StatusLED::init(&eventManager);
#endif

    eventManager.queueEvent(GPS_STATUS_CHANGED, 0);
    
}

void loop() {
    // put your main code here, to run repeatedly:
    eventManager.processEvent();
    
    if (millis() > 6000 && millis() < 7000) {
        eventManager.queueEvent(GPS_STATUS_CHANGED, 1);
    }
    serialEvent();
    yield();
}

void serialEvent() {
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        serialBuffer += inChar;

        if (inChar == '\n') {
            message = serialBuffer;
            serialBuffer = "";
            
            eventManager.queueEvent(GPS_MESSAGE_RECEIVED, 1);
        }
    }
}

