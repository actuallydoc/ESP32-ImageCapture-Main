// BARVE
const uint16_t COLOR_WHITE = 0xFFFF;
const uint16_t COLOR_BLACK = 0x2945;
const uint16_t COLOR_GREEN = 0xB6E9;
const uint16_t COLOR_SILVER = 0xCE79;
const uint16_t COLOR_SILVER_DARK = 0xAD55;
const uint16_t COLOR_RED = 0xEBAE;
const uint16_t COLOR_BLUE = 0x7E9C;


// Gumbi
int BTN_SUBTYPE = 0;
int BTN_SUBTYPE_PREVIOUS = 0;
int BTN_SUBTYPE_PRESS_COUNTER = 0;
int DEVICE_STATUS_COUNT = 0;

const bool WATCHDOG_RESTART = true;
const bool INTERVAL_WATCHDOG_RESTART = true;
const unsigned long INTERVAL_BTN_SUBTYPE_UNSELECT = 5000; // miliseconds
/* external module config & pins */
// TouchScreen ILI9486
const int TOUCHSCREEN_TFT_CS = 5;
const int TOUCHSCREEN_TFT_DC = 4;
const int TOUCHSCREEN_TFT_LED = 0;
const int TOUCHSCREEN_TFT_MOSI = 23;
const int TOUCHSCREEN_TFT_CLK = 18;
const int TOUCHSCREEN_TFT_RST = 22;
const int TOUCHSCREEN_TFT_MISO = 19;
const int TOUCHSCREEN_TS_CS = 14;
const int TOUCHSCREEN_TS_IRQ = 27;
const int TOUCHSCREEN_TS_MINPRESSURE = 1;
const int TOUCHSCREEN_TS_MINX = 370;
const int TOUCHSCREEN_TS_MINY = 470;
const int TOUCHSCREEN_TS_MAXX = 3700;
const int TOUCHSCREEN_TS_MAXY = 3600;

// Modul SC
const int EXPANDER_PIN = 21; // Reset board pin
