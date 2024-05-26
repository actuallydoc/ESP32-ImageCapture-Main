/*********************************************************************
   @brief TouchScreen functions for drawing and output data on screen
 *********************************************************************/

// #define ILI9486_DRIVER


#include <TFT_eSPI.h>  // Hardware-specific library
#define ILI9488_DRIVER

// global variables
TFT_eSPI tft = TFT_eSPI();
int TouchScreenPositionX, TouchScreenPositionY;
// TS_Point p;

// constructor for Container
struct ContainerInit {
  float X;
  float Y;
  float Width;
  float Height;
  uint16_t backgroundColor;
} ContainerInitParam;

// constructor for Content
struct ContainerContent {
  // Text properties
  String Text;
  int FontSize;
  uint16_t FontColor;

  // Icon / Image properties
  const unsigned char* Icon;
  uint16_t IconColor;
  const short unsigned int* Image;
  float Width;
  float Height;
} ContainerContentParam[10];


class TouchScreenFunctions {
private:
  int DefaultFontHeight = 7;  // 5x7 default font size on setSize(1)
  int DefaultLineSpacing = 7;

public:
  int screenWidth = 320;
  int screenHeight = 480;
  int ScreenOrientation;

  void setup() {
    tft.begin();
    rotation();

    tft.fillScreen(COLOR_WHITE);

    backlight();
    
  }

  void backlight(bool turnOn = true) {
  }

  void rotation() {
    tft.setRotation(ScreenOrientation);

    if (ScreenOrientation == 2) {
      screenWidth = 320;
      screenHeight = 480;
    } else if (ScreenOrientation == 3) {
      screenWidth = 480;
      screenHeight = 320;
    }
  }

  void drawContainer(struct ContainerInit* ContainerInitParam, struct ContainerContent* ContainerContentParam) {
    int MediaWidth, MediaHeight = 0;

    int xPosition = (ContainerInitParam->X > 1) ? ContainerInitParam->X : ContainerInitParam->X * screenWidth;
    int yPosition = (ContainerInitParam->Y > 1) ? ContainerInitParam->Y : ContainerInitParam->Y * screenHeight;
    int widthCalculated = (ContainerInitParam->Width > 1) ? screenWidth : ContainerInitParam->Width * screenWidth;
    int heightCalculated = (ContainerInitParam->Height > 1) ? screenHeight : (ContainerInitParam->Y + ContainerInitParam->Height > 1) ? ((1 - ContainerInitParam->Y) * screenHeight) + 1
                                                                                                                                      : ContainerInitParam->Height * screenHeight;

    // draw main frame
    tft.invertDisplay(false);
    tft.fillRect(xPosition, yPosition, widthCalculated, heightCalculated, ContainerInitParam->backgroundColor);
    int TotalContentHeight = 0;

    // calculate height of Media element - should be always on first place
    bool Media = (ContainerContentParam[0].Icon || ContainerContentParam[0].Image) ? true : false;
    if (Media) {
      TotalContentHeight = ContainerContentParam[0].Height;

      MediaWidth = ContainerContentParam[0].Width;
      MediaHeight = ContainerContentParam[0].Height;
    }

    // calculate height of Text elements
    int LineSpacing = 0;
    int TotalElements = 0;
    for (int i = 0; i < 10; i++) {
      if (ContainerContentParam[i].Text != "") {
        int FontSize = ContainerContentParam[i].FontSize * DefaultFontHeight;
        LineSpacing = (i == 0) ? 0 : ContainerContentParam[i].FontSize * DefaultLineSpacing;

        TotalContentHeight += FontSize + LineSpacing;

        TotalElements++;
      }
    }

    int yStartPosition = calculateContainerCenter(yPosition, heightCalculated, TotalContentHeight);

    // draw media - Image or Icon
    if (Media) {
      int xMediaPosition = calculateContainerCenter(xPosition, widthCalculated, MediaWidth);
      int yMediaPosition = yStartPosition;

      if (ContainerContentParam[0].Icon) {
        tft.drawBitmap(xMediaPosition, yMediaPosition, ContainerContentParam[0].Icon, MediaWidth, MediaHeight, ContainerContentParam[0].IconColor);
      } else if (ContainerContentParam[0].Image) {
        // fix when Media width is bigger than Screen width
        xMediaPosition = (MediaWidth > screenWidth) ? -((MediaWidth - screenWidth) / 2) : xMediaPosition;

        tft.setSwapBytes(true);
        tft.pushImage(xMediaPosition, yMediaPosition, MediaWidth, MediaHeight, ContainerContentParam[0].Image);
      }
    }

    // output text each row (text row should be from key 1 ahead)
    yStartPosition = (MediaHeight) ? yStartPosition + MediaHeight : yStartPosition;
    LineSpacing = 0;
    for (int i = 0; i < 10; i++) {
      if (ContainerContentParam[i].Text != "") {
        int FontSize = ContainerContentParam[i].FontSize * DefaultFontHeight;
        LineSpacing = (i == 0) ? 0 : FontSize + (ContainerContentParam[i - 1].FontSize * DefaultLineSpacing);

        yStartPosition += LineSpacing;

        drawText(ContainerContentParam[i].Text, ContainerContentParam[i].FontSize, xPosition, yStartPosition, widthCalculated);
      }
    }
  }

  void drawButton(float positionX, float positionY, float width, float height, uint16_t backgroundColor, String text) {
    int x, y = 0;
    int widthCalculated = width * screenWidth;
    int heightCalculated = height * screenHeight;

    x = positionX * screenWidth;
    y = positionY * screenHeight;

    tft.fillRect(x, y, widthCalculated, heightCalculated, backgroundColor);

    int IconWidth = 40;
    int IconHeight = 40;

    int TextWidth = 25;
    int TextHeight = 25;

    int xIconPosition = calculateContainerCenter(x, widthCalculated, IconWidth);
    int yIconPosition = calculateContainerCenter(y, heightCalculated, IconHeight + TextHeight);

    drawText(text, 1, x, yIconPosition + IconHeight + 20, widthCalculated);
  }

  void drawText(String text, int textSize, int x, int y, int widthCalculated) {
    tft.invertDisplay(false);
    tft.setTextSize(textSize);
    int xTextPosition = calculateContainerCenter(x, widthCalculated, 0);
    tft.setTextColor(COLOR_WHITE);
    tft.drawCentreString(text, xTextPosition, y, 1);
  }

  int calculateContainerCenter(int elementStart, int containerSize, int elementSize) {
    int output = 0;

    output = ((elementStart + (elementStart + containerSize)) / 2) - (elementSize / 2);

    return output;
  }

  void clear() {
    // ContainerInitParam = { 0, 0, 240, 320, COLOR_WHITE };
    ContainerInitParam = {};
    for (int i = 0; i < 10; i++) {
      ContainerContentParam[i].Text = "";
      ContainerContentParam[i].FontSize = 0;
      ContainerContentParam[i].Icon = NULL;
      ContainerContentParam[i].Image = NULL;
      ContainerContentParam[i].Width = 0;
      ContainerContentParam[i].Height = 0;
    }

    drawContainer(&ContainerInitParam, ContainerContentParam);
    tft.fillRect(0, 0, 320, 350, COLOR_WHITE); // flickering bug
  }

};
TouchScreenFunctions TouchScreen;
