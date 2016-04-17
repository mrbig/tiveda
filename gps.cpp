#include "gps.h"

// Pointer to the incoming serial message buffer
String* GPS::message;

// Current status: true if we have reception
boolean GPS::reception;

// We keep the last valid state from gps here
GPSSTATUS GPS::last;

// Reference to the eventManager
EventManager* GPS::eventManager;

/**
 * Parse incoming messages. This gets called once a complete
 * message is ready in the message propertu
 */
void GPS::handleMessage(int eventCode, int eventParam) {
    if (!calculateChecksum()) {
#ifdef DEBUG
        Serial.print("Checksum error");
#endif
        return;
    }
    if (message->startsWith("$GPRMC,")) {
        Serial.print("> ");
        Serial.println(*message);
        parseRMC(message);
    }
}

/**
 * Handle GPRMC messages
 */
void GPS::parseRMC(String* msg) {
    byte offset = 7;
    byte fieldNr = 0;
    String current;
    boolean old;

    float rawCoord, rawSpd, rawHdg;
    char rawIndicator;

    while (fieldNr < 8) {
        current = msg->substring(offset);
        switch (fieldNr) {
            case 1:
                old = reception;
                if (current.charAt(0) == 'A') {
                    reception = true;
                    last.timestamp = millis();
                } else if (current.charAt(0) == 'V') {
                    reception = false;
                }
                if (old != reception) {
                    eventManager->queueEvent(GPS_STATUS_CHANGED, reception);
                }
                break;

            case 2:
            case 4:
                rawCoord = current.toFloat();
                break;

            case 3:
            case 5:
                if (!rawCoord) break;
                rawIndicator = current.charAt(0);

                if (fieldNr == 3) {
                    last.lat = toCoordinates(rawCoord, rawIndicator);
                } else {
                    last.lng = toCoordinates(rawCoord, rawIndicator);
                }

                break;

            case 6:
                if (current.indexOf(',') == 0) {
                    last.spd = -1;
                    break;
                }
                last.spd = current.toFloat() * 1.852;
                break;

            case 7:
                if (current.indexOf(',') == 0) {
                    last.hdg = -1;
                    break;
                }
                last.hdg = current.toFloat();
        }
        
        offset = msg->indexOf(',', offset) + 1;
        fieldNr++;
    }

    Serial.print("Lat: ");
    Serial.println(last.lat,6);
    Serial.print("Lng: ");
    Serial.println(last.lng,6);
    Serial.print("SPD: ");
    Serial.println(last.spd);
    Serial.print("HDG: ");
    Serial.println(last.hdg);
}

/**
 * Convert NMEA coordinates to defimal representation
 * @param float the coordinate in ddmm.mmmm format
 * @param char the N/s/E/W indicator
 * @return float the converted value
 */
float GPS::toCoordinates(float coord, char indicator) {
    byte deg;
    float min, dec;

    deg = coord / 100;
    min = coord - (deg * 100);
    
    dec = deg + min/60;
   
    if (indicator == 'W' || indicator == 'S') {
        dec = - dec;
    }

    return dec;
   
}

/**
 * Calculate the checksum for the current NMEA sequence
 * @returns 1 on success 0 else
 */
byte GPS::calculateChecksum() {
    byte i, last;
    uint16_t checksum;

    last = message->lastIndexOf('*');
    if (last < 0) return 0;

    checksum = 0;
    for (i=1; i<last; i++) {
        checksum = checksum ^ message->charAt(i);
    }
    
    return message->substring(last + 1, message->length()-1) == String(checksum, HEX);
}

