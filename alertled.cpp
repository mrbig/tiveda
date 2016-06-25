#include "alertled.h"

// Current alert status
int8_t AlertLED::alertStatus;

// Position in the animation
uint8_t AlertLED::animPtr;

// Ticker
Ticker AlertLED::ticker;

// Timer used in GPS on/off animation
Ticker AlertLED::receptionTicker;

// Position in reception sound
uint16_t AlertLED::receptionCounter = 0;

// Current reception status
byte AlertLED::receptionStatus = 0;

// Inverted mode
bool AlertLED::inverted = false;

/**
 * Register event handlers, and start timer
 * @param EventManager the event manager used for events
 * @param bool wether we are in inverted mode.
 */
void AlertLED::init(EventManager* eventManager, bool isInverted) {
    inverted = isInverted;
    
    pinMode(CFG_LED_ALERT1, OUTPUT);
    pinMode(CFG_LED_ALERT2, OUTPUT);
    pinMode(CFG_BEEPER, OUTPUT);
    digitalWrite(CFG_LED_ALERT1, 1 - inverted);
    digitalWrite(CFG_LED_ALERT2, 1 - inverted);
    digitalWrite(CFG_BEEPER, 1);
    
    alertStatus = ALERT_NONE;
    
    eventManager->addListener(ALERT_TRIGGERED, &AlertLED::alertTriggeredCallback);
    eventManager->addListener(ALERT_RESET, &AlertLED::resetCallback);
    eventManager->addListener(GPS_STATUS_CHANGED, &AlertLED::resetCallback);
    ticker.attach_ms(50, &AlertLED::tickCallback);
};


/**
 * Incoming alert update
 */
void AlertLED::alertTriggeredCallback(int eventCode, int eventParam) {
    Serial.print("Alert: ");
    Serial.println(eventParam);
    if (eventParam <= -10) {
        alertStatus = ALERT_INFO1;
    }
    else if (eventParam < -5) {
        alertStatus = ALERT_INFO2;
    }
    else if (eventParam <= 0) {
        alertStatus = ALERT_INFO3;
    }
    else if (eventParam <= 15) {
        alertStatus = ALERT_WARNING;
    }
    else {
        alertStatus = ALERT_DANGER;
    }
    if (alertStatus == ALERT_WARNING) {
        // Warning state is constantly speeding up
        ticker.attach_ms(50-3*eventParam, &AlertLED::tickCallback);
    } else {
        ticker.attach_ms(50, &AlertLED::tickCallback);
    }
};

/**
 * Alert has been reset
 */
void AlertLED::resetCallback(int eventCode, int eventParam) {
    if (eventCode != GPS_STATUS_CHANGED || eventParam == 0) {
        alertStatus = ALERT_NONE;
    }
    if (eventCode == GPS_STATUS_CHANGED) {
        if (eventParam == receptionStatus) return;
        receptionStatus = eventParam;
        // Start anim
        if (receptionStatus) {
            receptionCounter = 1;
        } else {
            receptionCounter = 10;
        }
        analogWriteFreq(receptionCounter * 88);
        analogWrite(CFG_BEEPER, 512);
        receptionTicker.attach_ms(100, &AlertLED::receptionAnimCallback);
    }
};

/**
 * When GPS reception changed this callback plays the anim
 */
void AlertLED::receptionAnimCallback() {
    if (receptionStatus) {
        receptionCounter++;
    } else {
        receptionCounter--;
    }
    
    analogWriteFreq(receptionCounter * 88);
    
    if (receptionCounter == 10 || receptionCounter == 1) {
        // End of anim
        receptionTicker.detach();
        analogWrite(CFG_BEEPER, 1023);
    }
}

