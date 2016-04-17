#include "gps.h"

String* GPS::message;

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
        Serial.print(*message);
    }
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

