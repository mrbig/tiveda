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
#ifdef DEBUG
    Serial.print("Setting status to ");
    Serial.print(state);
    Serial.print("\n");
#endif

    gpsStatus = state;

    pos = 0; // reset animation

}

/**
 * Handle ticks
 */
void StatusLED::tick() {
    if (!gpsStatus) {
        // Display sweep animation whene there is no reception
        analogWrite(CFG_LED_STATUS, inverted ? 1023 - animPulse[pos] : animPulse[pos]);
        pos++;
        if (pos >= sizeof(animPulse) / 2) pos = 0;
    } else {
        // Short pings when reception acquired
        if (pos == 0) {
            if (GPS::isNight()) {
                analogWrite(CFG_LED_STATUS, inverted ? 123 : 900);
            } else {
                analogWrite(CFG_LED_STATUS, inverted ? 1023 : 1);
            }
        }
        else analogWrite(CFG_LED_STATUS, inverted ? 0 : 1023);
        if (++pos >= 150) pos = 0;
    }
}

