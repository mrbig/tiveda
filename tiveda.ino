
#include "config.h"
#include <FS.h>
#include <EventManager.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>


//*************************************************************************************
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <ESP_SSD1306.h>
#define OLED_RESET  16  // Pin 15 -RESET digital signal
ESP_SSD1306 display(OLED_RESET); // FOR I2C

//*************************************************************************************




#include "gps.h"
#include "poi.h"

// Current version
const char VERSION[] PROGMEM = FW_VERSION;
const char BVERSION[] PROGMEM = BOARD_VERSION;

#ifdef ENABLE_STATUSLED
#include "statusled.h"
#endif

#ifdef ENABLE_ALERTLED
#include "alertled.h"
#endif

/**
 * Global event manager instance
 */
EventManager eventManager;

/**
 * Global wifi manager instance
 */
WiFiManager wifiManager;

// Buffer to collect incoming data on serial
String serialBuffer = "";

// Holds the last complete line received
String message;

// We keep the map here
POI* pois;

// Map's current version
uint32_t mapVersion;

// Number of the pois
uint16_t poiCount;

// Set to true when we're in an alert
boolean inAlert = false;

// This is set true while we're waiting for wifi
boolean wifiConnecting = false;

// Wether this board uses active high output for leds
boolean inverted = false;
/**
 * System initialization
 */
void setup() {
    // Init serial communication
    Serial.begin(9600);
    //*************************************************************************************
    initDisplay();
    //*************************************************************************************

    delay(2000);


    serialBuffer.reserve(200);
    message.reserve(200);

    // inverted = isBoardInverted();
#ifdef DEBUG
    if (inverted) {
        Serial.println("Board is inverted");
    } else {
        Serial.println("Board is normal");
    }
#endif

    // Setup PWM frequency
    analogWriteFreq(880);

    // Init wifi
    wifiConnecting = true;
#ifndef DEBUG
    wifiManager.setDebugOutput(false);
#endif
    wifiManager.setNonBlocking(true);
    wifiManager.setConnectTimeout(30);
    wifiManager.setConfigPortalTimeout(600);
    wifiManager.autoConnect("tiveda");

    // Load map and start gps event handlers
    loadMap();
    
    GPS::init(&eventManager, &message);

    // Initialize output modules
#ifdef ENABLE_STATUSLED
    StatusLED::init(&eventManager, inverted);
#endif
#ifdef ENABLE_ALERTLED
    AlertLED::init(&eventManager, inverted);
#endif

    // First events
    eventManager.queueEvent(GPS_STATUS_CHANGED, 0);
    eventManager.addListener(GPS_UPDATED, &checkPois);
    
}

/**
 * Main event loop. Basically we just process the events from the eventManager
 */
void loop() {
    // put your main code here, to run repeatedly:
    eventManager.processEvent();
    // WifiManager events
    if (WiFi.getMode() != WIFI_OFF) {
        wifiManager.process();
    }
    // Seems like this does not get called on ESP8266
    serialEvent();
    // Check for wifi connection
    if (wifiConnecting && WiFi.status() == WL_CONNECTED) {
        performOTA();
    }
    yield();
}

/**
 * Loading the map into memory
 */
void loadMap() {
    char buff[32];
    byte i,j, rred;

    poiCount = 0;
    
    SPIFFS.begin();
#ifdef DEBUG
    FSInfo fs_info;
    SPIFFS.info(fs_info);
    Serial.print(F("TotalBytes: "));
    Serial.println(fs_info.totalBytes);
    Serial.print(F("UsedBytes: "));
    Serial.println(fs_info.usedBytes);
    Serial.print(F("BlockSize: "));
    Serial.println(fs_info.blockSize);
    Serial.print(F("PageSize: "));
    Serial.println(fs_info.pageSize);

    //*************************************************************************************

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(F("TotalBytes: "));
    display.println(fs_info.totalBytes);
    display.print(F("UsedBytes: "));
    display.println(fs_info.usedBytes);
    display.print(F("BlockSize: "));
    display.println(fs_info.blockSize);
    display.print(F("PageSize: "));
    display.println(fs_info.pageSize);
    display.display();
    delay(2000);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.display();

    //*************************************************************************************



#endif

    File f;

    // Load current mapVersion
    if (SPIFFS.exists(F("/version.dat"))) {
        f = SPIFFS.open(F("/version.dat"), "r");
        f.readBytes(buff, sizeof(mapVersion));
        memcpy(&mapVersion, buff, sizeof(mapVersion));
        f.close();
    }

    f = SPIFFS.open(F("/map.dat"), "r");
    if (!f) {
#ifdef DEBUG
        Serial.println("Map open failed");
        display.println("Map open failed"); display.display();
        return;
#endif
    }

    // Read header
    f.readBytes(buff, 4);
    if (strncmp(buff, "POIS", 4)) {
#ifdef DEBUG
        Serial.println("Invalid or corrupt map file");
        display.println("Invalid or corrupt map file"); display.display();
        return;
#endif
    }

    // Read poi count
    f.readBytes(buff, 2);
    memcpy(&poiCount, buff, 2);

#ifdef DEBUG
    Serial.print("Loading map");
    display.print("Loading map"); display.display();
#endif

    pois = new POI[poiCount];

    for (i=0; i<poiCount; i++) {

        // Reading the header
        rred = f.readBytes(buff, 8);
        if (rred != 8) {
#ifdef DEBUG
            Serial.println("Short read, map might be corrupted");
            display.println("Short read, map might be corrupted");
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
                display.println("Short read, map might be corrupted"); display.display();
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
    Serial.print("Map version: ");
    Serial.println(mapVersion, HEX);
    Serial.print("Loaded ");
    Serial.print(poiCount);
    Serial.print(" pois, free heap: ");
    Serial.println(ESP.getFreeHeap());

    //*************************************************************************************
    display.clearDisplay();
    display.setCursor(0, 0);
    // display.clearDisplay();
    // display.print("\n");

    display.println("Map version: ");
    display.setTextSize(2);
    display.println(mapVersion, HEX);
    display.setTextSize(1);
    display.println("Loaded ");
    display.setTextSize(2);
    display.print(poiCount);
    display.println(" pois");
    display.setTextSize(1);
    display.print("free heap: ");
    display.println(ESP.getFreeHeap());
    display.display();
    delay(5000);
    display.clearDisplay();
    display.display();

    //*************************************************************************************

#endif

}



/**
 * Handle incoming data from the serial
 */
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

       //*************************************************************************************
        display.setTextSize(2);
        display.setCursor(35, 49);
        // display.setTextColor(BLACK);
        display.print(pois[i].limit); display.print("km/h");
        //  display.setTextColor(WHITE);
        display.display();
        //*************************************************************************************



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

/**
 * Perform OTA update
 */
void performOTA() {
    String url = String(F("http://ota.sneaker.hu/?id=")) + String(ESP.getChipId(), HEX) + String(F("&board=")) + FPSTR(BVERSION);
    if (inverted) {
        url += "i";
    } else {
        url += "n";
    }
    
    wifiConnecting = false;
    
#ifdef DEBUG
    Serial.println("");
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
    
    Serial.print(F("Starting OTA..."));

    //*************************************************************************************
    display.setCursor(0, 0);
    display.println("");
    display.println(F("WiFi connected"));
    display.println(F("IP address: "));
    display.println(WiFi.localIP());
    display.print(F("Starting OTA..."));
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();

    //*************************************************************************************


#endif

    // Update application code
    t_httpUpdate_return ret = ESPhttpUpdate.update(url, FPSTR(VERSION));
    checkErrors(ret);

#ifdef DEBUG
    Serial.print(F("Updating map..."));
    display.print(F("Updating map...")); display.display();
#endif
    // Update application SPIFFS
    ret = ESPhttpUpdate.updateSpiffs(url, String(mapVersion, HEX));
    checkErrors(ret);



    WiFi.mode(WIFI_OFF); // Turn off wifi to save on power
}

/**
 *  Parse errors from httpupdate
 */
void checkErrors(t_httpUpdate_return ret) {
    switch (ret) {
        case HTTP_UPDATE_FAILED:
#ifdef DEBUG
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
#endif;
            break;

        case HTTP_UPDATE_NO_UPDATES:
#ifdef DEBUG
            Serial.println(F("no update found"));

            //*************************************************************************************
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println(F("no update found"));
            display.display();
            delay(2000);
            display.clearDisplay();
            display.display();

            //*************************************************************************************

#endif
            break;

        case HTTP_UPDATE_OK:
#ifdef DEBUG
            Serial.println(F("update successfull"));

            //*************************************************************************************
            display.setCursor(0, 0);
            display.println(F("update successfull"));
            display.display();
            delay(2000);
            display.clearDisplay();
            display.display();

            //*************************************************************************************

#endif
            break;
    }
}


/**
 *  Check if the current board is inverted
 *  D8 and D1 should be connected. We set D1 to various
 *  states and check if D8 does match
 *  
 *  This method blocks for ~50ms
 *  @return bool true if the board matches the criteria
 */
bool isBoardInverted() {
#ifdef INVERT_OUT
    pinMode(INVERT_OUT, OUTPUT);
    pinMode(INVERT_IN, INPUT);

    digitalWrite(INVERT_OUT, HIGH);
    delay(10);
    if (digitalRead(INVERT_IN) != HIGH) return false;
    digitalWrite(INVERT_OUT, LOW);
    delay(10);
    if (digitalRead(INVERT_IN) != LOW) return false;
    digitalWrite(INVERT_OUT, HIGH);
    delay(10);
    if (digitalRead(INVERT_IN) != HIGH) return false;
    digitalWrite(INVERT_OUT, LOW);
    delay(10);
    if (digitalRead(INVERT_IN) != LOW) return false;

    pinMode(INVERT_OUT, INPUT);
    return true;
#else
    return false;
#endif
}

