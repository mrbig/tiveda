#include "gps.h"

String* GPS::message;

/**
 * Parse incoming messages. This gets called once a complete
 * message is ready in the message propertu
 */
void GPS::handleMessage(int eventCode, int eventParam) {
    Serial.print("> ");
    Serial.print(*message);
}

