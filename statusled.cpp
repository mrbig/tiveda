#include "statusled.h"

/**
 * This is the singleton instance
 */
StatusLED* StatusLED::instance;

/**
 * GPS Status has been changed
 */
void StatusLED::statusChanged(int eventCode, int eventParam) {
    Serial.print("Status changed called to ");
    Serial.print(eventParam);
    Serial.print("\n");
}



