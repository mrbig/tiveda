
#include "poi.h"


/**
 * Check if point is inside of the polygon
 * @param float latitude
 * @param float longitude
 * @return true if the point is inside
 */
boolean POI::checkPointInside(float lat, float lng) {
    EDGE* edge;
    byte oddNodes = 0;
    byte i;

    for (i=0; i<edgeCount; i++) {
        edge = &edges[i];
        if ((edge->lng1 < lng && lng <= edge->lng2)
            || (edge->lng2 < lng && lng <= edge->lng1))
        {
            oddNodes ^= (lng * edge->m + edge->c) < lat;
        }
    }
    return oddNodes;
}

