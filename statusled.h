#ifndef H_STATUSLED
#define H_STATUSLED

#include "config.h"
#include <EventManager.h>

class StatusLED {
    public:
        static StatusLED* getInstance() {return StatusLED::instance;};

        /**
         * Register event handlers, and start timer
         * @param EventManager the event manager used for events
         */
        static void init(EventManager* eventManager) {
            instance = new StatusLED();

            eventManager->addListener(GPS_STATUS_CHANGED, &StatusLED::statusChanged);
        };

        /**
         * GPS Status has been changed
         */
         static void statusChanged(int eventCode, int eventParam);

    protected:
        // This is the singleton instance
        static StatusLED* instance;


};

#endif


