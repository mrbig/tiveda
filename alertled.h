#ifndef H_ALERTLED
#define H_ALERTLED

#include "config.h"
#include <EventManager.h>
#include <Ticker.h>
#include "alertled_anim.h"
#include "gps.h"

// Alert levels
#define ALERT_NONE -1
#define ALERT_INFO1 0
#define ALERT_INFO2 1
#define ALERT_INFO3 2
#define ALERT_WARNING 3
#define ALERT_DANGER 4


/**
 * Use two leds (left, right) to show various alert pattern when close 
 * to a poi
 */
class AlertLED {
    public:

        /**
         * Register event handlers, and start timer
         * @param EventManager the event manager used for events
         * @param bool wether we should work in inverted mode
         */
        static void init(EventManager* eventManager, bool isInverted);

        /**
         * Progress the current animation
         */
        static void tickCallback() {
            ALED_CFG anim;
            float mult = GPS::isNight() ? 0.05 : 1;

            if (alertStatus == ALERT_NONE) {
                if (!animPtr) return;
                animPtr = 0;
                analogWrite(CFG_LED_ALERT1, inverted ? 0 : 1023);
                analogWrite(CFG_LED_ALERT2, inverted ? 0 : 1023);
                analogWrite(CFG_BEEPER, 1023);
                return;
            }

            if (animPtr >= sizeof(anim_warning[alertStatus])/sizeof(ALED_CFG)) animPtr = 0;

            memcpy_P(&anim, &anim_warning[alertStatus][animPtr], sizeof(ALED_CFG));
            if (inverted) {
                analogWrite(CFG_LED_ALERT1, (1024 -anim.lft) * mult);
                analogWrite(CFG_LED_ALERT2, (1024 - anim.rgt) * mult);
            } else {
                analogWrite(CFG_LED_ALERT1, (anim.lft - 1023) * mult + 1023);
                analogWrite(CFG_LED_ALERT2, (anim.rgt - 1023) * mult + 1023);
            }
            analogWrite(CFG_BEEPER, anim.beeper);
            
            animPtr++;
        }

        /**
         * When GPS reception changed this callback plays the anim
         */
        static void receptionAnimCallback();

        /**
         * Incoming alert update
         */
        static void alertTriggeredCallback(int eventCode, int eventParam);

        /**
         * Alert has been reset
         */
        static void resetCallback(int eventCode, int eventParam);

    protected:
        /**
         * Should we work in inverted mode?
         */
        static bool inverted;
        /**
         * Set to the current alert status
         */
        static int8_t alertStatus;

        /**
         * Current position in the animation
         */
        static uint8_t animPtr;

        // Timer used for event timing
        static Ticker ticker;

        // Timer used in GPS on/off animation
        static Ticker receptionTicker;

        // Position in reception sound
        static uint16_t receptionCounter;

        // Current reception status
        static byte receptionStatus;

};

#endif


