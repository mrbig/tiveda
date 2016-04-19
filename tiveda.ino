
#include "config.h"
#include <FS.h>
#include <EventManager.h>

#include "gps.h"
#include "poi.h"

#ifdef ENABLE_STATUSLED
#include "statusled.h"
#endif

#ifdef ENABLE_ALERTLED
#include "alertled.h"
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

// Set to true when we're in an alert
boolean inAlert = false;

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
#ifdef ENABLE_ALERTLED
    AlertLED::init(&eventManager);
#endif

    eventManager.queueEvent(GPS_STATUS_CHANGED, 0);
    eventManager.addListener(GPS_UPDATED, &checkPois);
    
}

/**
 * Loading the map into memory
 */
void loadMap() {
    char buff[32];
    byte i,j, rred;

    poiCount = 0;
    
    SPIFFS.begin();
    File f = SPIFFS.open("/map.dat", "r");
    if (!f) {
#ifdef DEBUG
        Serial.println("Map open failed");
        return;
#endif
    }

    f.readBytes(buff, 4);
    if (strncmp(buff, "POIS", 4)) {
#ifdef DEBUG
        Serial.println("Invalid or corrupt map file");
        return;
#endif
    }

    // Read poi count
    f.readBytes(buff, 2);
    memcpy(&poiCount, buff, 2);

#ifdef DEBUG
    Serial.print("Loading ");
#endif

    pois = new POI[poiCount];

    for (i=0; i<poiCount; i++) {

        // Reading the header
        rred = f.readBytes(buff, 8);
        if (rred != 8) {
#ifdef DEBUG
            Serial.println("Short read, map might be corrupted");
#endif
            poiCount = i - 1;
            if (poiCount < 1) poiCount = 0;
            goto finish;
        }
        memcpy(&pois[i], buff, 8);

        pois[i].edges = new EDGE[pois[i].edgeCount];
        
        // reading edges
        for (j=0; j<pois[i].edgeCount; j++) {
            rred = f.readBytes(buff, sizeof(EDGE));
            if (rred < sizeof(EDGE)) {
#ifdef DEBUG
                Serial.println("Short read, map might be corrupted");
#endif
                poiCount = i - 1;
                if (poiCount < 1) poiCount = 0;
                goto finish;
            }
            memcpy(&pois[i].edges[j], buff, sizeof(EDGE));

        }
        Serial.print(".");
    }
finish:

    f.close();

#ifdef DEBUG
    Serial.print("\n");
    Serial.print("Loaded ");
    Serial.print(poiCount);
    Serial.print(" pois, free heap: ");
    Serial.println(ESP.getFreeHeap());
#endif

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
    boolean alertFound = false;
#ifdef DEBUG
    unsigned long start = micros();
#endif
    
    current = GPS::getCurrent();
    
    for (i=0; i<poiCount; i++){
        if (!pois[i].checkPointInside(current->lat, current->lng)) continue;

        // check heading
        if (pois[i].heading > -0.5) {
            diff = abs(pois[i].heading - current->hdg);
            if (diff > 180) {
                diff = abs(diff - 360);
            }

            if (diff > CFG_HDG_TOLERANCE) continue;
        }

        // We trigger this each time, becouse the speed varies
        eventManager.queueEvent(ALERT_TRIGGERED, current->spd - pois[i].limit);
        alertFound = inAlert = true;
    }
    if (inAlert && !alertFound) {
        eventManager.queueEvent(ALERT_RESET, 0);
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

