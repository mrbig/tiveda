#include "alertled.h"

// Current alert status
int8_t AlertLED::alertStatus;

// Position in the animation
uint8_t AlertLED::animPtr;

// Ticker
Ticker AlertLED::ticker;

/**
 * Register event handlers, and start timer
 * @param EventManager the event manager used for events
 */
void AlertLED::init(EventManager* eventManager) {
    pinMode(CFG_LED_ALERT1, OUTPUT);
    pinMode(CFG_LED_ALERT2, OUTPUT);
    digitalWrite(CFG_LED_ALERT1, 1);
    digitalWrite(CFG_LED_ALERT2, 1);
    
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
};
