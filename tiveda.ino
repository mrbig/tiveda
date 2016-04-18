
#include "config.h"
#include <EventManager.h>

#include "gps.h"
#include "poi.h"

#ifdef ENABLE_STATUSLED
#include "statusled.h"
#endif

EventManager eventManager;

// Buffer to collect incoming data on serial
String serialBuffer = "";

// Holds the last complete line received
String message;

// We keep the map here
POI* pois;

// Number of the pois
uint8_t poiCount;


void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.setDebugOutput(true);
    serialBuffer.reserve(200);
    message.reserve(200);

    loadMap();
    
    GPS::init(&eventManager, &message);

#ifdef ENABLE_STATUSLED
    StatusLED::init(&eventManager);
#endif

    eventManager.queueEvent(GPS_STATUS_CHANGED, 0);
    eventManager.addListener(GPS_UPDATED, &checkPois);
    
}

/**
 * Loading the map into memory
 */
void loadMap() {
    poiCount = 2;
    
    pois = new POI[poiCount];

    pois[0].limit = 20;
    pois[0].heading = -1;
    pois[0].edgeCount = 6;
    pois[0].edges = new EDGE[pois[0].edgeCount];
    pois[0].edges[0] = {19.257617, 19.2570215, 0.759865659113, 32.9248752653};
    pois[0].edges[1] = {19.2570215, 19.2574614, -0.658331438952, 60.235127274};
    pois[0].edges[2] = {19.2574614, 19.2576277, 0.674684305537, 34.5646280289};
    pois[0].edges[3] = {19.2576277, 19.2573166, -0.581806493057, 58.7616600367};
    pois[0].edges[4] = {19.2573166, 19.2577779, 0.74550184261, 33.201263191};
    pois[0].edges[5] = {19.2577779, 19.257617, -0.652579241748, 60.1251981997};

    pois[1].limit = 50;
    pois[1].heading = 232;
    pois[1].edgeCount = 4;
    pois[1].edges = new EDGE[pois[1].edgeCount];
    pois[1].edges[0] = {19.1219669, 19.1221708, -0.674840608121, 60.4538778713};
    pois[1].edges[1] = {19.1221708, 19.1260117, 0.750344971231, 33.2012358012};
    pois[1].edges[2] = {19.1260117, 19.1257757, -0.644491525456, 59.8788949564};
    pois[1].edges[3] = {19.1257757, 19.1219669, 0.760475740394, 33.0078061639};

}

/**
 * Main event loop. Basically we just process the events from the eventManager
 */
void loop() {
    // put your main code here, to run repeatedly:
    eventManager.processEvent();
    // Seems like this does not get called on ESP8266
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

/**
 * Go throug the pois, and check if we're in any of those
 */
void checkPois(int eventCode, int eventParam) {
    uint8_t i;
    GPSSTATUS* current;
    float diff;
#ifdef DEBUG
    unsigned long start = micros();
#endif
    
    current = GPS::getCurrent();
    
    for (i=0; i<poiCount; i++){
        if (!pois[i].checkPointInside(current->lat, current->lng)) continue;

        if (pois[i].heading > -0.5) {
            diff = abs(pois[i].heading - current->hdg);
            if (diff > 180) {
                diff = abs(diff - 360);
            }

            if (diff > CFG_HDG_TOLERANCE) continue;
        }

        eventManager.queueEvent(ALERT_CHANGED, abs(current->spd - pois[i].limit));
#ifdef DEBUG
//        Serial.print("Matched poi #");
//        Serial.println(i);
#endif
    }
#ifdef DEBUG
    unsigned long finish = micros();
    Serial.print("Took: ");
    Serial.print((finish - start));
    Serial.print("us ");
    Serial.print("Free heep: ");
    Serial.println(ESP.getFreeHeap());
#endif;
}

