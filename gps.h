#ifndef H_GPS
#define H_GPS

#include <Arduino.h>
#include "config.h"
#include <EventManager.h>

struct GPSSTATUS {
    // Latitude
    float lat;
    // Longitude
    float lng;
    // Heading
    float hdg;
    // Speed
    float spd;
    // Timestamp of last valid gps data
    unsigned long timestamp;
};

class GPS {
    public:

        /**
         * Register event handlers
         * @param EventManager the event manager used for events
         */
        static void init(EventManager* em, String* buff) {
            eventManager = em;
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

        // true, if we have reception
        static boolean reception;

        // Latest latitude
        static GPSSTATUS last;

        // Reference to the event manager
        static EventManager* eventManager;
        

        /**
         * Calculate the checksum of the NMEA sentence
         */
        static byte calculateChecksum();

        /**
         * Parse GPRMC messages
         */
        static void parseRMC(String* msg);

        /**
         * Convert NMEA coordinates to defimal representation
         * @param float the coordinate in ddmm.mmmm format
         * @param char the N/s/E/W indicator
         * @return float the converted value
         */
        static float toCoordinates(float coord, char inverse);

};

#endif

