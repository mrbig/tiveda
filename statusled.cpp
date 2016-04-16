#include "statusled.h"

/**
 * This is the singleton instance
 */
StatusLED* StatusLED::instance;

/**
 * Constructor: start the ticker
 */
StatusLED::StatusLED() {
    ticker.attach_ms(100, &StatusLED::tickCallback);
}


/**
 * Change the GPS status
 */
void StatusLED::setStatus(byte state) {

    if (state == gpsStatus) return;
    Serial.print("Setting status to ");
    Serial.print(state);
    Serial.print("\n");

    gpsStatus = state;

    pos = 0; // reset animation

}

/**
 * Handle ticks
 */
void StatusLED::tick() {
    if (!gpsStatus) {
        // Display sweep animation whene there is no reception
        analogWrite(CFG_LED_STATUS, animPulse[pos]);
        pos++;
        if (pos >= sizeof(animPulse) / 2) pos = 0;
    } else {
        // Short pings when reception acquired
        analogWrite(CFG_LED_STATUS, 0);
        if (pos == 0) digitalWrite(CFG_LED_STATUS, 0);
        else digitalWrite(CFG_LED_STATUS, 1);
        if (++pos >= 150) pos = 0;
    }
}

