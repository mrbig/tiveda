#ifndef H_STATUSLED
#define H_STATUSLED

#include "config.h"
#include <EventManager.h>
#include <Ticker.h>
#include "gps.h"

class StatusLED {
    public:
        static StatusLED* getInstance() {return StatusLED::instance;};

        /**
         * Register event handlers, and start timer
         * @param EventManager the event manager used for events
         */
        static void init(EventManager* eventManager) {
            instance = new StatusLED();

            eventManager->addListener(GPS_STATUS_CHANGED, &StatusLED::statusChangedCallback);
        };

        /**
         * GPS Status has been changed
         */
        static void statusChangedCallback(int eventCode, int eventParam) {getInstance()->setStatus(eventParam);};

         /**
          * Change the GPS status
          */
         void setStatus(byte state);

         /**
          * Callback on timer ticks
          */
         static void tickCallback() {getInstance()->tick();};

    protected:
        // Constructor
        StatusLED();
        
        // This is the singleton instance
        static StatusLED* instance;

        // Status of the gps
        byte gpsStatus = -1;

        // Timer used for event timing
        Ticker ticker;

        /**
         * Handler timer ticks
         */
        void tick();

        /**
         * Position in the animation
         */
        word pos = 0;
        
        /**
         * Map for the pulse animation
         */
        const uint16_t animPulse[25] =  {1023, 980, 900, 800, 600, 400, 200, 0, 200, 400, 600, 800, 900, 1000, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};

};

#endif


