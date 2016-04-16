
#include "config.h"
#include <EventManager.h>

#ifdef ENABLE_STATUSLED
#include "statusled.h"
#endif

EventManager eventManager;




void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);

#ifdef ENABLE_STATUSLED
    StatusLED::init(&eventManager);
#endif

    eventManager.queueEvent(GPS_STATUS_CHANGED, 0);
    
}

void loop() {
    // put your main code here, to run repeatedly:
    eventManager.processEvent();
    yield();
}
