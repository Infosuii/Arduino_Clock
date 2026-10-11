/*
 An example analogue clock using a TFT LCD screen to show the time
 use of some of the drawing commands with the ST7735 library.

 For a more accurate clock, it would be better to use the RTClib library.
 But this is just a demo.

 Uses compile time to set the time so a reset will start with the compile time again

 Gilchrist 6/2/2014 1.0
 Updated by Bodmer
 Adapted for the LILYGO TTGO T-Display (135 x 240).
 Original: https://github.com/Bodmer/TFT_eSPI/tree/master/examples/160%20x%20128/TFT_Clock
 */

#include <Arduino.h>
#include <cmath>
#include <TFT_eSPI.h> // ST7789 and pins configured in platformio.ini
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

constexpr uint16_t CLOCK_BACKGROUND = 0xBDF7;

float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;    // Saved H, M, S x & y multipliers
float sdeg=0, mdeg=0, hdeg=0;
uint16_t osx=67, osy=90, omx=67, omy=90, ohx=67, ohy=90;  // Saved H, M, S x & y coords
uint16_t x0=0, x1=0, yy0=0, yy1=0;
uint32_t targetTime = 0;                    // for next 1 second timeout

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

uint8_t hh=conv2d(__TIME__), mm=conv2d(__TIME__+3), ss=conv2d(__TIME__+6);  // Get H, M, S from compile time

bool initial = 1;

void setup(void) {
  tft.init();
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  tft.setRotation(0);
  tft.fillScreen(CLOCK_BACKGROUND);
  tft.setTextColor(TFT_GREEN, CLOCK_BACKGROUND);  // Adding a black background colour erases previous text automatically

  // Draw clock face
  tft.fillCircle(67, 90, 61, TFT_BLUE);
  tft.fillCircle(67, 90, 57, TFT_BLACK);

  // Draw 12 lines
  for(int i = 0; i<360; i+= 30) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*57+67;
    yy0 = sy*57+90;
    x1 = sx*50+67;
    yy1 = sy*50+90;

    tft.drawLine(x0, yy0, x1, yy1, TFT_BLUE);
  }

  // Draw 60 dots
  for(int i = 0; i<360; i+= 6) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*53+67;
    yy0 = sy*53+90;

    tft.drawPixel(x0, yy0, TFT_BLUE);
    if(i==0 || i==180) tft.fillCircle(x0, yy0, 1, TFT_CYAN);
    if(i==0 || i==180) tft.fillCircle(x0+1, yy0, 1, TFT_CYAN);
    if(i==90 || i==270) tft.fillCircle(x0, yy0, 1, TFT_CYAN);
    if(i==90 || i==270) tft.fillCircle(x0+1, yy0, 1, TFT_CYAN);
  }

  tft.fillCircle(67, 90, 3, TFT_RED);

  // Place the caption below the clock face using font 4.
  // Only font numbers 2,4,6,7 are valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : . a p m
  // Font 7 is a 7 segment font and only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : .
  tft.drawCentreString("Time flies",67,165,4);

  targetTime = millis() + 1000;
}

void loop() {
  if (static_cast<int32_t>(millis() - targetTime) >= 0) {
    targetTime += 1000;
    ss++;              // Advance second
    if (ss==60) {
      ss=0;
      mm++;            // Advance minute
      if(mm>59) {
        mm=0;
        hh++;          // Advance hour
        if (hh>23) {
          hh=0;
        }
      }
    }

    // Pre-compute hand degrees, x & y coords for a fast screen update
    sdeg = ss*6;                  // 0-59 -> 0-354
    mdeg = mm*6+sdeg*0.01666667;  // 0-59 -> 0-360 - includes seconds
    hdeg = hh*30+mdeg*0.0833333;  // 0-11 -> 0-360 - includes minutes and seconds
    hx = cos((hdeg-90)*0.0174532925);
    hy = sin((hdeg-90)*0.0174532925);
    mx = cos((mdeg-90)*0.0174532925);
    my = sin((mdeg-90)*0.0174532925);
    sx = cos((sdeg-90)*0.0174532925);
    sy = sin((sdeg-90)*0.0174532925);

    if (ss==0 || initial) {
      initial = 0;
      // Erase hour and minute hand positions every minute
      tft.drawLine(ohx, ohy, 67, 90, TFT_BLACK);
      ohx = hx*33+67;
      ohy = hy*33+90;
      tft.drawLine(omx, omy, 67, 90, TFT_BLACK);
      omx = mx*44+67;
      omy = my*44+90;
    }

      // Redraw new hand positions, hour and minute hands not erased here to avoid flicker
      tft.drawLine(osx, osy, 67, 90, TFT_BLACK);
      tft.drawLine(ohx, ohy, 67, 90, TFT_WHITE);
      tft.drawLine(omx, omy, 67, 90, TFT_WHITE);
      osx = sx*47+67;
      osy = sy*47+90;
      tft.drawLine(osx, osy, 67, 90, TFT_RED);

    tft.fillCircle(67, 90, 3, TFT_RED);
  }
}
