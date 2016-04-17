#ifndef H_GPS
#define H_GPS

#include <Arduino.h>
#include "config.h"
#include <EventManager.h>

class GPS {
    public:

        /**
         * Register event handlers
         * @param EventManager the event manager used for events
         */
        static void init(EventManager* eventManager, String* buff) {
            eventManager->addListener(GPS_MESSAGE_RECEIVED, &GPS::handleMessage);
            message = buff;
        };

        /**
         * Handle incoming gps message
         */
        static void handleMessage(int eventCode, int eventParam);


    protected:
        // Reference to the message buffer
        static String* message;

        /**
         * Calculate the checksum of the NMEA sentence
         */
        static byte calculateChecksum();

};

#endif

