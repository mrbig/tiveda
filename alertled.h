#ifndef H_ALERTLED
#define H_ALERTLED

#include "config.h"
#include <EventManager.h>
#include <Ticker.h>
#include "alertled_anim.h"

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
         */
        static void init(EventManager* eventManager);

        /**
         * Progress the current animation
         */
        static void tickCallback() {
            ALED_CFG anim;

            if (alertStatus == ALERT_NONE) {
                if (!animPtr) return;
                animPtr = 0;
                analogWrite(CFG_LED_ALERT1, 1023);
                analogWrite(CFG_LED_ALERT2, 1023);
                analogWrite(CFG_BEEPER, 1023);
                return;
            }

            if (animPtr >= sizeof(anim_warning[alertStatus])/sizeof(ALED_CFG)) animPtr = 0;

            memcpy_P(&anim, &anim_warning[alertStatus][animPtr], sizeof(ALED_CFG));
            analogWrite(CFG_LED_ALERT1, anim.lft);
            analogWrite(CFG_LED_ALERT2, anim.rgt);
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


