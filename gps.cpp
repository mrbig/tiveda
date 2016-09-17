#include "gps.h"
#include "sat.h"


int ora;
String perc;
String masodperc;
String datum;

// Pointer to the incoming serial message buffer
String* GPS::message;

// Current status: true if we have reception
boolean GPS::reception;

// We keep the last valid state from gps here
GPSSTATUS GPS::last;

// Reference to the eventManager
EventManager* GPS::eventManager;

/**
 * Parse incoming messages. This gets called once a complete
 * message is ready in the message propertu
 */
void GPS::handleMessage(int eventCode, int eventParam) {
    if (!calculateChecksum()) {
#ifdef DEBUG
        Serial.print("X ");
        Serial.print(*message);
        Serial.println("Checksum error");
#endif
        return;
    }
    if (message->startsWith("$GPRMC,")) {
#ifdef DEBUG
        Serial.print("> ");
        Serial.println(*message);
#endif
        parseRMC(message);
    }
}

/**
 * Handle GPRMC messages
 */
void GPS::parseRMC(String* msg) {
    byte offset = 7;
    byte fieldNr = 0;
    String current;
    boolean old;

    float rawCoord, rawSpd, rawHdg;
    char rawIndicator;

    while (fieldNr < 9) {
        current = msg->substring(offset);
        switch (fieldNr) {
            // UTC Time
            case 0:
                last.time = current.substring(0,2).toInt() * 60 + current.substring(2,4).toInt();
                //*****************************************************************************
                ora = current.substring(0, 2).toInt() + 1;
                if (ora == 24) ora = 0;
                perc = current.substring(2, 4) ;
                masodperc = current.substring(4, 6) ;
                //****************************************************************************
                break;

            // Status
            case 1:
                old = reception;
                if (current.charAt(0) == 'A') {
                    reception = true;
                    last.timestamp = millis();
                } else if (current.charAt(0) == 'V') {
                    reception = false;
                }
                if (old != reception) {
                    eventManager->queueEvent(GPS_STATUS_CHANGED, reception);
                }
                break;

            case 2: // Latitude
            case 4: // Longitude
                rawCoord = current.toFloat();
                break;

            case 3: // N/S Indicator
            case 5: // E/W Indicator
                if (!rawCoord) break;
                rawIndicator = current.charAt(0);

                if (fieldNr == 3) {
                    last.lat = toCoordinates(rawCoord, rawIndicator);
                } else {
                    last.lng = toCoordinates(rawCoord, rawIndicator);
                }

                break;

            // Speed Over Ground
            case 6:
                if (current.indexOf(',') == 0) {
                    last.spd = -1;
                    break;
                }
                last.spd = current.toFloat() * 1.852;
                break;

            // Course Over Ground
            case 7:
                if (current.indexOf(',') == 0) {
                    last.hdg = -1;
                    break;
                }
                last.hdg = current.toFloat();

            // Date
            case 8:
                last.day = current.substring(0, 2).toInt() + (current.substring(2, 4).toInt() - 1) * 30;
                datum = current.substring(4, 6) + current.substring(2, 4) + current.substring(0, 2);
                break;
        }
        
        offset = msg->indexOf(',', offset) + 1;
        fieldNr++;
    }

    if (reception) {
        eventManager->queueEvent(GPS_UPDATED, 0);
    }

#ifdef DEBUG
    Serial.print("Lat: ");
    Serial.println(last.lat,6);
    Serial.print("Lng: ");
    Serial.println(last.lng,6);
    Serial.print("SPD: ");
    Serial.println(last.spd);
    Serial.print("HDG: ");
    Serial.println(last.hdg);
    Serial.print("Is night: ");
    Serial.println(isNight());
    Serial.print("ido: ");
    Serial.print(ora); Serial.print(":"); Serial.print(perc); Serial.print(":"); Serial.println(masodperc);
    Serial.print(" last.day: ");
    Serial.println(last.day);
    Serial.print("Datum: ");
    Serial.println(datum);
#endif

    //***********************************OLED**************************************************

    display.setCursor(30, 25);
    display.setTextSize(3);
    display.fillRect(0, 0, 128, 64, BLACK);

    if (last.spd > 0)
    {
        if (last.spd < 100)
        {
            display.print(" ");
        }
        if (last.spd < 10)
        {
            display.print(" ");
        }

        if (last.spd < 1)
        {
            display.println("0.0");
        }
        else
        {
            display.println(last.spd, 1);
        }


        if (perc != ",,")
        {
            if (datum.toInt() > 160327 && datum.toInt() < 161030) ora = ora + 1;
            if (datum.toInt() > 170326 && datum.toInt() < 171029) ora = ora + 1;
            if (datum.toInt() > 180325 && datum.toInt() < 181028) ora = ora + 1;
            if (datum.toInt() > 190331 && datum.toInt() < 191027) ora = ora + 1;
            if (datum.toInt() > 200329 && datum.toInt() < 201025) ora = ora + 1;
            if (datum.toInt() > 210328 && datum.toInt() < 211031) ora = ora + 1;
            if (datum.toInt() > 220327 && datum.toInt() < 221030) ora = ora + 1;
            if (datum.toInt() > 230326 && datum.toInt() < 231029) ora = ora + 1;
            if (datum.toInt() > 240331 && datum.toInt() < 241027) ora = ora + 1;
            if (datum.toInt() > 250330 && datum.toInt() < 251026) ora = ora + 1;
            if (datum.toInt() > 260327 && datum.toInt() < 261030) ora = ora + 1;
            if (ora == 24) ora = 0;

            display.setCursor(20, 0);
            display.setTextSize(2);
            //  display.fillRect(0, 0, 128, 30, BLACK);
            if (ora < 10)  display.setCursor(25, 0);
            display.print(ora); display.print(":"); display.print(perc); display.print(":"); display.println(masodperc);
        }
    }
    else
    {
        display.setCursor(22, 0);
        display.fillRect(0, 0, 128, 64, BLACK);
        // display.clearDisplay();
        display.setTextSize(1);
        display.println("no GPS signal");
        display.drawBitmap(40, 10, sat, 48, 48, WHITE);
        display.display();
    }

    display.display();

    //*************************************************************************************


}

/**
 * Returns true if we're after sunset
 * @return boolean true when it's night at the moment
 */
boolean GPS::isNight() {
    SUNINFO today;
    if (last.day < 0 || last.day > 359) return false;
    memcpy_P(&today, &sunrisemap[last.day], sizeof(SUNINFO));
    return last.time < today.sunrise || today.sunset < last.time;
}

/**
 * Convert NMEA coordinates to defimal representation
 * @param float the coordinate in ddmm.mmmm format
 * @param char the N/s/E/W indicator
 * @return float the converted value
 */
float GPS::toCoordinates(float coord, char indicator) {
    byte deg;
    float min, dec;

    deg = coord / 100;
    min = coord - (deg * 100);
    
    dec = deg + min/60;
   
    if (indicator == 'W' || indicator == 'S') {
        dec = - dec;
    }

    return dec;
   
}

/**
 * Calculate the checksum for the current NMEA sequence
 * @returns 1 on success 0 else
 */
byte GPS::calculateChecksum() {
    byte i, last;
    uint16_t checksum;
    String strChecksum;

    last = message->lastIndexOf('*');
    if (last < 0) return 0;

    checksum = 0;
    for (i=1; i<last; i++) {
        checksum = checksum ^ message->charAt(i);
    }
    strChecksum = String(checksum, HEX);
    strChecksum.toUpperCase();
    if (checksum < 16) {
        strChecksum = "0" + strChecksum;
    }
    
    return message->substring(last + 1, last + 3) == strChecksum;
}

