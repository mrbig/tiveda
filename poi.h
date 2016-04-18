#ifndef POI_H
#define POI_H

#include <Arduino.h>
#include "config.h"

struct EDGE {
    // First longitude
    float lng1;
    // Last longitude
    float lng2;
    // Multiplier
    float m;
    // Constant
    float c;
};

class POI {
    public:
        // Speed limit
        uint8_t limit;

        // Heading limit, if less than 0, then we don't use this value
        float heading;

        // Number of edges in this poi
        uint8_t edgeCount;

        // Edges of the polygon
        EDGE* edges;

        /**
         * Check if point is inside of the polygon
         * @param float latitude
         * @param float longitude
         * @return true if the point is inside
         */
        boolean checkPointInside(float lat, float lng);
};

#endif
