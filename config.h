
// Define this for various debug features
#define DEBUG 1

// Enable / disabled modules
#define ENABLE_STATUSLED 1
#define ENABLE_ALERTLED 1

// GPIO port of the status led
#define CFG_LED_STATUS 2

// GPIO port of the alert led
#define CFG_LED_ALERT1 14

// GPIO port of the debug button
#define CFG_BTN 0

// Tolerance in heading from the requiered in degress
#define CFG_HDG_TOLERANCE 45

// Custom events
#define GPS_STATUS_CHANGED EventManager::kEventUser1

// New message is available
#define GPS_MESSAGE_RECEIVED EventManager::kEventUser2

// GPS coordinates have been updated
#define GPS_UPDATED EventManager::kEventUser3

// Alert status has been changed
// Message parameter is the difference between the current speed and the speed limit
#define ALERT_TRIGGERED EventManager::kEventUser4

// Alert is gone
#define ALERT_RESET EventManager::kEventUser5

