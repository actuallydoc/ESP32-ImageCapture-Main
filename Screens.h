#include "esp32-hal-gpio.h"
SC16IS752 I2CExpander = SC16IS752(SC16IS750_PROTOCOL_I2C, SC16IS750_ADDRESS_AA);
#define minimum(a, b) (((a) < (b)) ? (a) : (b))

void renderButton(int position, bool active = false) {
  if (position == -1) {
    BTN_SUBTYPE = (active) ? -1 : 0;

      if (active) {
        BTN_SUBTYPE_PRESS_COUNTER = (BTN_SUBTYPE == BTN_SUBTYPE_PREVIOUS) ? BTN_SUBTYPE_PRESS_COUNTER + 1 : 0;
      }

      BTN_SUBTYPE_PREVIOUS = 1;
  } 
  if (position == 4) {
      BTN_SUBTYPE = (active) ? 4 : 0;

      if (active) {
        BTN_SUBTYPE_PRESS_COUNTER = (BTN_SUBTYPE == BTN_SUBTYPE_PREVIOUS) ? BTN_SUBTYPE_PRESS_COUNTER + 1 : 0;
      }
  
      BTN_SUBTYPE_PREVIOUS = 4;
      String buttonName = "Izbrisi";
      TouchScreen.drawButton(0.025, 0.715, 0.3, 0.27, (active == true) ? COLOR_GREEN : COLOR_SILVER_DARK, buttonName);
    } else if (position == 6) {
      BTN_SUBTYPE = (active) ? 6 : 0;

      if (active) {
        BTN_SUBTYPE_PRESS_COUNTER = (BTN_SUBTYPE == BTN_SUBTYPE_PREVIOUS) ? BTN_SUBTYPE_PRESS_COUNTER + 1 : 0;
      }

      BTN_SUBTYPE_PREVIOUS = 6;
      String buttonName = "Zajemi sliko";
      TouchScreen.drawButton(0.675, 0.715, 0.3, 0.27, (active == true) ? COLOR_GREEN : COLOR_SILVER_DARK, buttonName);
    }
}
void HomeScreen() {
  TouchScreen.clear();
  // Image
  ContainerContentParam[0] = {};
  TouchScreen.drawContainer(&ContainerInitParam, ContainerContentParam);

  BTN_SUBTYPE_PREVIOUS = 0;
  BTN_SUBTYPE_PRESS_COUNTER = 0;
  
  renderButton(4);
  renderButton(6);
}


void izbrisiGumb() {
   TouchScreen.clear();
  // Image
  ContainerContentParam[0] = {};
  TouchScreen.drawContainer(&ContainerInitParam, ContainerContentParam);

  BTN_SUBTYPE_PREVIOUS = 0;
  BTN_SUBTYPE_PRESS_COUNTER = 0;
  
  renderButton(4);
  renderButton(6);
  IZBRISI = false;
}

void NastaviEkran() {
  if (WATCHDOG_RESTART) {
    esp_task_wdt_init(INTERVAL_WATCHDOG_RESTART, true);
    esp_task_wdt_add(NULL);
  }

  I2CExpander.pinMode(TOUCHSCREEN_TFT_LED, OUTPUT);
  I2CExpander.digitalWrite(TOUCHSCREEN_TFT_LED, LOW);
  TouchScreen.ScreenOrientation = 2;
  TouchScreen.setup();
  HomeScreen();
}
void ButtonPressed() {
  // unset previous button
  if (BTN_SUBTYPE_PREVIOUS > 0) {
    renderButton(BTN_SUBTYPE_PREVIOUS);
  }

  // ScreenOrientation
  if (TouchScreen.ScreenOrientation == 2) {  // portrait mode
    if (TouchScreenPositionY > 280) {
        if (TouchScreenPositionX > 240) {  // Gumb izbriši sliko
          if (!IZBRISI) {
            IZBRISI = true; // Nastavi izbriši na true
            renderButton(4, true); // Nastavi, da je gumb izbriši aktiven.
          }
        } else if (TouchScreenPositionX < 120) {  // Gumb zajemi sliko
          if (!ZAJEMI) {
            ZAJEMI = true; // Nastavi zajemi na true
            renderButton(6, true); // Nastavi, da je gumb zajemi slko aktiven.
          }
        }
      }
  }

}


void renderJPEG(int xpos, int ypos)
{

    // retrieve information about the image
    uint16_t *pImg;
    uint16_t mcu_w = JpegDec.MCUWidth;
    uint16_t mcu_h = JpegDec.MCUHeight;
    uint32_t max_x = JpegDec.width;
    uint32_t max_y = JpegDec.height;

    uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
    uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

    uint32_t win_w = mcu_w;
    uint32_t win_h = mcu_h;

    max_x += xpos;
    max_y += ypos;
    
    while (JpegDec.read())
    {

        pImg = JpegDec.pImage;

        int mcu_x = JpegDec.MCUx * mcu_w + xpos; // Calculate coordinates of top left corner of current MCU
        int mcu_y = JpegDec.MCUy * mcu_h + ypos;
        
        if (mcu_x + mcu_w <= max_x)
            win_w = mcu_w;
        else
            win_w = min_w;

        // check if the image block size needs to be changed for the bottom edge
        if (mcu_y + mcu_h <= max_y)
            win_h = mcu_h;
        else
            win_h = min_h;

        // copy pixels into a contiguous block
        if (win_w != mcu_w)
        {
            uint16_t *cImg;
            int p = 0;
            cImg = pImg + win_w;
            for (int h = 1; h < win_h; h++)
            {
                p += mcu_w;
                for (int w = 0; w < win_w; w++)
                {
                    *cImg = *(pImg + w + p);
                    cImg++;
                }
            }
        }

        // calculate how many pixels must be drawn
        uint32_t mcu_pixels = win_w * win_h;

        tft.startWrite();

        // draw image MCU block only if it will fit on the screen
        if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
        {

            // Now set a MCU bounding window on the TFT to push pixels into (x, y, x + width - 1, y + height - 1)
            tft.setAddrWindow(mcu_x, mcu_y, win_w, win_h);

            // Write all MCU pixels to the TFT window
            while (mcu_pixels--)
            {
                // Push each pixel to the TFT MCU area
                tft.pushColor(*pImg++);
            }
        }
        else if ((mcu_y + win_h) >= tft.height())
            JpegDec.abort(); // Image has run off bottom of screen so abort decoding

        tft.endWrite();
    }
  
}

void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos)
{
    int x = xpos;
    int y = ypos;
    JpegDec.decodeArray(arrayname, array_size);
    renderJPEG(x, y);
}
