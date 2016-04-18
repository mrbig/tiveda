#ifndef H_ALERTLED
#define H_ALERTLED

#include "config.h"
#include <EventManager.h>
#include <Ticker.h>

class AlertLED {
    public:
        /**
         * Register event handlers, and start timer
         * @param EventManager the event manager used for events
         */
        static void init(EventManager* eventManager) {
            pinMode(CFG_LED_ALERT1, OUTPUT);
            digitalWrite(CFG_LED_ALERT1, 0);
            
            eventManager->addListener(ALERT_TRIGGERED, &AlertLED::alertTriggeredCallback);
            eventManager->addListener(ALERT_RESET, &AlertLED::resetCallback);
        };

        /**
         * Incoming alert update
         */
        static void alertTriggeredCallback(int eventCode, int eventParam) {
            Serial.print("Alert: ");
            Serial.println(eventParam);
            if (eventParam <= 0) {
                analogWrite(CFG_LED_ALERT1, 0);
                digitalWrite(CFG_LED_ALERT1, 1);
            } else {
                analogWriteFreq(1);
                analogWrite(CFG_LED_ALERT1, 512);
            }
        };

        /**
         * Alert has been reset
         */
        static void resetCallback(int eventCode, int eventParam) {
            analogWrite(CFG_LED_ALERT1, 0);
            digitalWrite(CFG_LED_ALERT1, 0);
        };


};

#endif


