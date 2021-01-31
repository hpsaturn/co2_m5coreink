#include <Arduino.h>
#include <M5CoreInk.h>
#include <Wire.h>
#include <SparkFun_SCD30_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_SCD30

// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>
// Note: if you use this with ENABLE_GxEPD2_GFX 1:
//       uncomment it in GxEPD2_GFX.h too, or add #include <GFX.h> before any #include <GxEPD2_GFX.h>

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
GxEPD2_154_M09 medp = GxEPD2_154_M09(/*CS=D8*/ 9, /*DC=D3*/ 15, /*RST=D4*/ 0, /*BUSY=D2*/ 4);
GxEPD2_BW<GxEPD2_154_M09, GxEPD2_154_M09::HEIGHT> display(medp); // GDEH0154D67

#if !defined(__AVR) && !defined(_BOARD_GENERIC_STM32F103C_H_)

// comment out unused bitmaps to reduce code space used
#include "bitmaps/Bitmaps200x200.h" // 1.54" b/w
#include "bitmaps/Bitmaps104x212.h" // 2.13" b/w flexible GDEW0213I5F
#include "bitmaps/Bitmaps128x250.h" // 2.13" b/w
#include "bitmaps/Bitmaps128x296.h" // 2.9"  b/w
#include "bitmaps/Bitmaps176x264.h" // 2.7"  b/w
#include "bitmaps/Bitmaps400x300.h" // 4.2"  b/w
#include "bitmaps/Bitmaps640x384.h" // 7.5"  b/w
// 3-color
#include "bitmaps/Bitmaps3c200x200.h" // 1.54" b/w/r
#include "bitmaps/Bitmaps3c104x212.h" // 2.13" b/w/r
#include "bitmaps/Bitmaps3c128x296.h" // 2.9"  b/w/r
#include "bitmaps/Bitmaps3c176x264.h" // 2.7"  b/w/r
#include "bitmaps/Bitmaps3c400x300.h" // 4.2"  b/w/r
#if defined(ESP8266) || defined(ESP32)
#include "bitmaps/WS_Bitmaps800x600.h" // 6.0"  grey
#endif
#endif

#define BUTTON_UP_PIN       37
#define BUTTON_DOWN_PIN     39
#define BUTTON_MID_PIN      38
#define BUTTON_EXT_PIN      5
#define BUTTON_PWR_PIN      27

#define LED_EXT_PIN         10

#define SPEAKER_PIN         2 
#define TONE_PIN_CHANNEL    0

#define POWER_HOLD_PIN      12

// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_BW.h, GxEPD2_3C.h or GxEPD2_GFX.h for method setPartialWindow()

void drawFont(const char name[], const GFXfont* f);

const char HelloWorld[] = "Hello World!";
const char HelloArduino[] = "Hello Arduino!";
const char HelloEpaper[] = "Hello E-Paper!";


SCD30 airSensor;


#ifdef _GxBitmaps200x200_H_
void drawBitmaps200x200()
{
#if defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    logo200x200, first200x200 //, second200x200, third200x200, fourth200x200, fifth200x200, sixth200x200, senventh200x200, eighth200x200
  };
#elif defined(_BOARD_GENERIC_STM32F103C_H_)
  const unsigned char* bitmaps[] =
  {
    logo200x200, first200x200, second200x200, third200x200, fourth200x200, fifth200x200 //, sixth200x200, senventh200x200, eighth200x200
  };
#else
  const unsigned char* bitmaps[] =
  {
    logo200x200, first200x200, second200x200, third200x200, fourth200x200, fifth200x200, sixth200x200, senventh200x200, eighth200x200
  };
#endif
  if ((display.epd2.panel == GxEPD2::GDEP015OC1) || (display.epd2.panel == GxEPD2::GDEH0154D67))
  {
    bool m = display.mirror(true);
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmaps[i], display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      display.display();
      delay(2000);
    }
    display.mirror(m);
  }
  //else
  {
    bool mirror_y = (display.epd2.panel != GxEPD2::GDE0213B1);
    display.clearScreen(); // use default for white
    int16_t x = (int16_t(display.epd2.WIDTH) - 200) / 2;
    int16_t y = (int16_t(display.epd2.HEIGHT) - 200) / 2;
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      display.drawImage(bitmaps[i], x, y, 200, 200, false, mirror_y, true);
      delay(2000);
    }
  }
  bool mirror_y = (display.epd2.panel != GxEPD2::GDE0213B1);
  for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
  {
    int16_t x = -60;
    int16_t y = -60;
    for (uint16_t j = 0; j < 10; j++)
    {
      display.writeScreenBuffer(); // use default for white
      display.writeImage(bitmaps[i], x, y, 200, 200, false, mirror_y, true);
      display.refresh(true);
      if (display.epd2.hasFastPartialUpdate)
      {
        // for differential update: set previous buffer equal to current buffer in controller
        display.epd2.writeScreenBufferAgain(); // use default for white
        display.epd2.writeImageAgain(bitmaps[i], x, y, 200, 200, false, mirror_y, true);
      }
      delay(2000);
      x += 40;
      y += 40;
      if ((x >= int16_t(display.epd2.WIDTH)) || (y >= int16_t(display.epd2.HEIGHT))) break;
    }
    if (!display.epd2.hasFastPartialUpdate) break; // comment out for full show
    break; // comment out for full show
  }
  display.writeScreenBuffer(); // use default for white
  display.writeImage(bitmaps[0], int16_t(0), 0, 200, 200, false, mirror_y, true);
  display.writeImage(bitmaps[0], int16_t(int16_t(display.epd2.WIDTH) - 200), int16_t(display.epd2.HEIGHT) - 200, 200, 200, false, mirror_y, true);
  display.refresh(true);
  delay(2000);
}
#endif

#ifdef _GxBitmaps104x212_H_
void drawBitmaps104x212()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    WS_Bitmap104x212, Bitmap104x212_1, Bitmap104x212_2, Bitmap104x212_3
  };
#else
  const unsigned char* bitmaps[] =
  {
    WS_Bitmap104x212, Bitmap104x212_1, Bitmap104x212_2, Bitmap104x212_3
  };
#endif
  if (display.epd2.panel == GxEPD2::GDEW0213I5F)
  {
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawBitmap(0, 0, bitmaps[i], display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      display.display();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps128x250_H_
void drawBitmaps128x250()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    Bitmap128x250_1, logo128x250, first128x250, second128x250, third128x250
  };
#else
  const unsigned char* bitmaps[] =
  {
    Bitmap128x250_1, logo128x250, first128x250, second128x250, third128x250
  };
#endif
  if ((display.epd2.panel == GxEPD2::GDE0213B1) || (display.epd2.panel == GxEPD2::GDEH0213B72))
  {
    bool m = display.mirror(true);
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmaps[i], display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      display.display();
      delay(2000);
    }
    display.mirror(m);
  }
}
#endif

#ifdef _GxBitmaps128x296_H_
void drawBitmaps128x296()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    Bitmap128x296_1, logo128x296, first128x296, second128x296, third128x296
  };
#else
  const unsigned char* bitmaps[] =
  {
    Bitmap128x296_1, logo128x296 //, first128x296, second128x296, third128x296
  };
#endif
  if (display.epd2.panel == GxEPD2::GDEH029A1)
  {
    bool m = display.mirror(true);
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmaps[i], display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      display.display();
      delay(2000);
    }
    display.mirror(m);
  }
}
#endif

#ifdef _GxBitmaps176x264_H_
void drawBitmaps176x264()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    Bitmap176x264_1, Bitmap176x264_2, Bitmap176x264_3, Bitmap176x264_4, Bitmap176x264_5
  };
#else
  const unsigned char* bitmaps[] =
  {
    Bitmap176x264_1, Bitmap176x264_2 //, Bitmap176x264_3, Bitmap176x264_4, Bitmap176x264_5
  };
#endif
  if (display.epd2.panel == GxEPD2::GDEW027W3)
  {
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmaps[i], display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      display.display();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps400x300_H_
void drawBitmaps400x300()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    Bitmap400x300_1, Bitmap400x300_2
  };
#else
  const unsigned char* bitmaps[] = {}; // not enough code space
#endif
  if (display.epd2.panel == GxEPD2::GDEW042T2)
  {
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmaps[i], display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      display.display();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps640x384_H_
void drawBitmaps640x384()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    Bitmap640x384_1, Bitmap640x384_2
  };
#else
  const unsigned char* bitmaps[] = {}; // not enough code space
#endif
  if ((display.epd2.panel == GxEPD2::GDEW075T8) || (display.epd2.panel == GxEPD2::GDEW075Z09))
  {
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmaps[i], display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      display.display();
      delay(2000);
    }
  }
}
#endif

#ifdef _WS_Bitmaps800x600_H_
void drawBitmaps800x600()
{
#if defined(ESP8266) || defined(ESP32)
  if (display.epd2.panel == GxEPD2::ED060SCT)
  {
    //    Serial.print("sizeof(WS_zoo_800x600) is "); Serial.println(sizeof(WS_zoo_800x600));
    display.drawNative(WS_zoo_800x600, 0, 0, 0, 800, 600, false, false, true);
    delay(2000);
    //    Serial.print("sizeof(WS_pic_1200x825) is "); Serial.println(sizeof(WS_pic_1200x825));
    //    display.drawNative((const uint8_t*)WS_pic_1200x825, 0, 0, 0, 1200, 825, false, false, true);
    //    delay(2000);
    //    Serial.print("sizeof(WS_acaa_1024x731) is "); Serial.println(sizeof(WS_acaa_1024x731));
    //    display.drawNative(WS_acaa_1024x731, 0, 0, 0, 1024, 731, false, false, true);
    //    delay(2000);
  }
#endif
}
#endif

struct bitmap_pair
{
  const unsigned char* black;
  const unsigned char* red;
};

#ifdef _GxBitmaps3c200x200_H_
void drawBitmaps3c200x200()
{
  bitmap_pair bitmap_pairs[] =
  {
    //{Bitmap3c200x200_black, Bitmap3c200x200_red},
    {WS_Bitmap3c200x200_black, WS_Bitmap3c200x200_red}
  };
  if (display.epd2.panel == GxEPD2::GDEW0154Z04)
  {
    display.fillScreen(GxEPD_WHITE);
    // Bitmap3c200x200_black has 2 bits per pixel
    // taken from Adafruit_GFX.cpp, modified
    int16_t byteWidth = (display.epd2.WIDTH + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;
    for (int16_t j = 0; j < display.epd2.HEIGHT; j++)
    {
      for (int16_t i = 0; i < display.epd2.WIDTH; i++)
      {
        if (i & 3) byte <<= 2;
        else
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          byte = pgm_read_byte(&Bitmap3c200x200_black[j * byteWidth * 2 + i / 4]);
#else
          byte = Bitmap3c200x200_black[j * byteWidth * 2 + i / 4];
#endif
        }
        if (!(byte & 0x80))
        {
          display.drawPixel(i, j, GxEPD_BLACK);
        }
      }
    }
    display.drawInvertedBitmap(0, 0, Bitmap3c200x200_red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
    display.display();
    delay(2000);
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      display.display();
      delay(2000);
    }
  }
  if (display.epd2.hasColor)
  {
    display.clearScreen(); // use default for white
    int16_t x = (int16_t(display.epd2.WIDTH) - 200) / 2;
    int16_t y = (int16_t(display.epd2.HEIGHT) - 200) / 2;
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.drawImage(bitmap_pairs[i].black, bitmap_pairs[i].red, x, y, 200, 200, false, false, true);
      delay(2000);
    }
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      int16_t x = -60;
      int16_t y = -60;
      for (uint16_t j = 0; j < 10; j++)
      {
        display.writeScreenBuffer(); // use default for white
        display.writeImage(bitmap_pairs[i].black, bitmap_pairs[i].red, x, y, 200, 200, false, false, true);
        display.refresh();
        delay(1000);
        x += 40;
        y += 40;
        if ((x >= int16_t(display.epd2.WIDTH)) || (y >= int16_t(display.epd2.HEIGHT))) break;
      }
    }
    display.writeScreenBuffer(); // use default for white
    display.writeImage(bitmap_pairs[0].black, bitmap_pairs[0].red, 0, 0, 200, 200, false, false, true);
    display.writeImage(bitmap_pairs[0].black, bitmap_pairs[0].red, int16_t(display.epd2.WIDTH) - 200, int16_t(display.epd2.HEIGHT) - 200, 200, 200, false, false, true);
    display.refresh();
    delay(2000);
  }
}
#endif

#ifdef _GxBitmaps3c104x212_H_
void drawBitmaps3c104x212()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c104x212_1_black, Bitmap3c104x212_1_red},
    {Bitmap3c104x212_2_black, Bitmap3c104x212_2_red},
    {WS_Bitmap3c104x212_black, WS_Bitmap3c104x212_red}
  };
#else
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c104x212_1_black, Bitmap3c104x212_1_red},
    //{Bitmap3c104x212_2_black, Bitmap3c104x212_2_red},
    {WS_Bitmap3c104x212_black, WS_Bitmap3c104x212_red}
  };
#endif
  if (display.epd2.panel == GxEPD2::GDEW0213Z16)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      if (bitmap_pairs[i].red == WS_Bitmap3c104x212_red)
      {
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      }
      else display.drawBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      display.display();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c128x296_H_
void drawBitmaps3c128x296()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c128x296_1_black, Bitmap3c128x296_1_red},
    {Bitmap3c128x296_2_black, Bitmap3c128x296_2_red},
    {WS_Bitmap3c128x296_black, WS_Bitmap3c128x296_red}
  };
#else
  bitmap_pair bitmap_pairs[] =
  {
    //{Bitmap3c128x296_1_black, Bitmap3c128x296_1_red},
    //{Bitmap3c128x296_2_black, Bitmap3c128x296_2_red},
    {WS_Bitmap3c128x296_black, WS_Bitmap3c128x296_red}
  };
#endif
  if (display.epd2.panel == GxEPD2::GDEW029Z10)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      if (bitmap_pairs[i].red == WS_Bitmap3c128x296_red)
      {
        display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      }
      else display.drawBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      display.display();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c176x264_H_
void drawBitmaps3c176x264()
{
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c176x264_black, Bitmap3c176x264_red}
  };
  if (display.epd2.panel == GxEPD2::GDEW027C44)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      display.drawBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      display.display();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c400x300_H_
void drawBitmaps3c400x300()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c400x300_1_black, Bitmap3c400x300_1_red},
    {Bitmap3c400x300_2_black, Bitmap3c400x300_2_red},
    {WS_Bitmap3c400x300_black, WS_Bitmap3c400x300_red}
  };
#else
  bitmap_pair bitmap_pairs[] = {}; // not enough code space
#endif
  if (display.epd2.panel == GxEPD2::GDEW042Z15)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      display.fillScreen(GxEPD_WHITE);
      display.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_BLACK);
      display.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, display.epd2.WIDTH, display.epd2.HEIGHT, GxEPD_RED);
      display.display();
      delay(2000);
    }
  }
}
#endif



void helloWorld()
{
  //Serial.println("helloWorld");
  // display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  // full window mode is the initial mode, set it anyway
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y);
  display.print(HelloWorld);
  display.display(false); // full update
  //Serial.println("helloWorld done");
}


void printCO2()
{
  //Serial.println("helloWorld");
  // display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  // full window mode is the initial mode, set it anyway
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y);
  display.print(HelloWorld);
  display.display(false); // full update
  //Serial.println("helloWorld done");
}

void helloWorldForDummiesNotPaged()
{
  //Serial.println("helloWorld");
  const char text[] = "Hello World!";
  // most e-papers have width < height (portrait) as native orientation, especially the small ones
  // in GxEPD2 rotation 0 is used for native orientation (most TFT libraries use 0 fix for portrait orientation)
  // set rotation to 1 (rotate right 90 degrees) to have enough space on small displays (landscape)
  display.setRotation(1);
  // select a suitable font in Adafruit_GFX
  display.setFont(&FreeMonoBold9pt7b);
  // on e-papers black on white is more pleasant to read
  display.setTextColor(GxEPD_BLACK);
  // Adafruit_GFX has a handy method getTextBounds() to determine the boundary box for a text for the actual font
  int16_t tbx, tby; uint16_t tbw, tbh; // boundary box window
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh); // it works for origin 0, 0, fortunately (negative tby!)
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  // full window mode is the initial mode, set it anyway
  display.setFullWindow();
  // here we use full buffered drawing
  // only part of the display is used if page_height is less than HEIGHT
  display.fillScreen(GxEPD_WHITE); // set the background to white (fill the buffer with value for white)
  display.setCursor(x, y); // set the postition to start printing text
  display.print(text); // print some text
  // tell the graphics class to transfer the buffer content (page) to the controller buffer
  // and to refresh to the screen
  display.display(false); // full update
  //Serial.println("helloWorld done");
}

void helloFullScreenPartialMode()
{
  //Serial.println("helloFullScreenPartialMode");
  const char fullscreen[] = "full screen update";
  const char fpm[] = "fast partial mode";
  const char spm[] = "slow partial mode";
  const char npm[] = "no partial mode";
  // full window mode is the initial mode, set it anyway
  display.setFullWindow();
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  const char* updatemode;
  if (display.epd2.hasFastPartialUpdate)
  {
    updatemode = fpm;
  }
  else if (display.epd2.hasPartialUpdate)
  {
    updatemode = spm;
  }
  else
  {
    updatemode = npm;
  }
  // do this outside of the loop
  int16_t tbx, tby; uint16_t tbw, tbh;
  // center update text
  display.getTextBounds(fullscreen, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t utx = ((display.width() - tbw) / 2) - tbx;
  uint16_t uty = ((display.height() / 4) - tbh / 2) - tby;
  // center update mode
  display.getTextBounds(updatemode, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t umx = ((display.width() - tbw) / 2) - tbx;
  uint16_t umy = ((display.height() * 3 / 4) - tbh / 2) - tby;
  // center HelloWorld
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t hwx = ((display.width() - tbw) / 2) - tbx;
  uint16_t hwy = ((display.height() - tbh) / 2) - tby;
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(hwx, hwy);
  display.print(HelloWorld);
  display.setCursor(utx, uty);
  display.print(fullscreen);
  display.setCursor(umx, umy);
  display.print(updatemode);
  display.display(true); // partial update
  //Serial.println("helloFullScreenPartialMode done");
}

void helloArduino()
{
  Serial.println("helloArduino");
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  // align with centered HelloWorld
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  // height might be different
  display.getTextBounds(HelloArduino, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t y = ((display.height() / 4) - tbh / 2) - tby; // y is base line!
  // make the window big enough to cover (overwrite) descenders of previous text
  uint16_t wh = FreeMonoBold9pt7b.yAdvance;
  uint16_t wy = (display.height() / 4) - wh / 2;
  // full window mode is the initial mode, set it anyway
  display.setFullWindow();
  // DO NOT display.fillScreen(GxEPD_WHITE); keep current content!
  display.fillRect(0, wy, display.width(), wh, GxEPD_WHITE); // clear rect instead
  display.setCursor(x, y);
  display.print(HelloArduino);
  display.displayWindow(0, wy, display.width(), wh);
  delay(1000);
  Serial.println("helloArduino done");
}

void helloEpaper()
{
  Serial.println("helloEpaper");
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  // align with centered HelloWorld
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  // height might be different
  display.getTextBounds(HelloEpaper, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t y = (display.height() * 3 / 4) + tbh / 2; // y is base line!
  // make the window big enough to cover (overwrite) descenders of previous text
  uint16_t wh = FreeMonoBold9pt7b.yAdvance;
  uint16_t wy = (display.height() * 3 / 4) - wh / 2;
  // full window mode is the initial mode, set it anyway
  display.setFullWindow();
  // DO NOT display.fillScreen(GxEPD_WHITE); keep current content!
  display.fillRect(0, wy, display.width(), wh, GxEPD_WHITE); // clear rect instead
  display.setCursor(x, y);
  display.print(HelloEpaper);
  display.displayWindow(0, wy, display.width(), wh);
  Serial.println("helloEpaper done");
}

#if defined(ESP8266) || defined(ESP32)
#include <StreamString.h>
#define PrintString StreamString
#else
class PrintString : public Print, public String
{
  public:
    size_t write(uint8_t data) override
    {
      return concat(char(data));
    };
};
#endif

void helloValue(double v, int digits)
{
  //Serial.println("helloValue");
  // display.setRotation(1);
  // display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
  PrintString valueString;
  valueString.print(v, digits);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(valueString, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = (display.height() * 3 / 4) + tbh / 2; // y is base line!
  // show what happens, if we use the bounding box for partial window
  uint16_t wx = (display.width() - tbw) / 2;
  uint16_t wy = (display.height() * 3 / 4) - tbh / 2;
  // full window mode is the initial mode, set it anyway
  display.setFullWindow();
  // DO NOT display.fillScreen(GxEPD_WHITE); keep current content!
  display.fillRect(wx, wy, tbw, tbh, GxEPD_WHITE); // clear rect instead
  display.setCursor(x, y);
  display.print(valueString);
  display.displayWindow(wx, wy, tbw, tbh);
  delay(2000);
  // make the partial window big enough to cover the previous text
  uint16_t ww = tbw; // remember window width
  display.getTextBounds(HelloEpaper, 0, 0, &tbx, &tby, &tbw, &tbh);
  // adjust, because HelloEpaper was aligned, not centered (could calculate this to be precise)
  ww = max(ww, uint16_t(tbw + 12)); // 12 seems ok
  wx = (display.width() - tbw) / 2;
  // make the window big enough to cover (overwrite) descenders of previous text
  uint16_t wh = FreeMonoBold9pt7b.yAdvance;
  wy = (display.height() * 3 / 4) - wh / 2;
  // DO NOT display.fillScreen(GxEPD_WHITE); keep current content!
  display.fillRect(wx, wy, ww, wh, GxEPD_WHITE); // clear rect instead
  display.setCursor(x, y);
  display.print(valueString);
  display.displayWindow(wx, wy, ww, wh);
  // alternately use the whole width for partial window
  //display.displayWindow(wx, wy, display.width(), wh);
  //Serial.println("helloValue done");
}

void deepSleepTest()
{
  Serial.println("deepSleepTest");
  const char hibernating[] = "hibernating ...";
  const char wokeup[] = "woke up";
  const char from[] = "from deep sleep";
  const char again[] = "again";
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  // center text
  display.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(x, y);
  display.print(hibernating);
  display.display(false); // full update
  display.hibernate();
  delay(5000);
  M5.shutdown(10);
  display.getTextBounds(wokeup, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t wx = (display.width() - tbw) / 2;
  uint16_t wy = (display.height() / 3) + tbh / 2; // y is base line!
  display.getTextBounds(from, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t fx = (display.width() - tbw) / 2;
  uint16_t fy = (display.height() * 2 / 3) + tbh / 2; // y is base line!
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(wx, wy);
  display.print(wokeup);
  display.setCursor(fx, fy);
  display.print(from);
  display.display(false); // full update
  M5.shutdown(10);
  // delay(5000);
  display.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t hx = (display.width() - tbw) / 2;
  uint16_t hy = (display.height() / 3) + tbh / 2; // y is base line!
  display.getTextBounds(again, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t ax = (display.width() - tbw) / 2;
  uint16_t ay = (display.height() * 2 / 3) + tbh / 2; // y is base line!
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(hx, hy);
  display.print(hibernating);
  display.setCursor(ax, ay);
  display.print(again);
  display.display(false); // full update
  display.hibernate();
  Serial.println("deepSleepTest done");
  M5.shutdown(10);
}

void showBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool partial)
{
  //Serial.println("showBox");
  display.setRotation(1);
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.fillRect(x, y, w, h, GxEPD_BLACK);
  if (partial)
  {
    display.displayWindow(x, y, w, h);
  }
  else
  {
    display.display(false); // full update
  }
  //Serial.println("showBox done");
}

void drawCornerTest()
{
  display.setFullWindow();
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  for (uint16_t r = 0; r <= 4; r++)
  {
    display.setRotation(r);
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(0, 0, 8, 8, GxEPD_BLACK);
    display.fillRect(display.width() - 18, 0, 16, 16, GxEPD_BLACK);
    display.fillRect(display.width() - 25, display.height() - 25, 24, 24, GxEPD_BLACK);
    display.fillRect(0, display.height() - 33, 32, 32, GxEPD_BLACK);
    display.setCursor(display.width() / 2, display.height() / 2);
    display.print(display.getRotation());
    display.display(false); // full update
    delay(2000);
  }
}

void showFont(const char name[], const GFXfont* f)
{
  display.setFullWindow();
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  drawFont(name, f);
  display.display(false); // full update
}

void drawFont(const char name[], const GFXfont* f)
{
  //display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(0, 0);
  display.println();
  display.println(name);
  display.println(" !\"#$%&'()*+,-./");
  display.println("0123456789:;<=>?");
  display.println("@ABCDEFGHIJKLMNO");
  display.println("PQRSTUVWXYZ[\\]^_");
  if (display.epd2.hasColor)
  {
    display.setTextColor(GxEPD_RED);
  }
  display.println("`abcdefghijklmno");
  display.println("pqrstuvwxyz{|}~ ");
}

// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_BW.h, GxEPD2_3C.h or GxEPD2_GFX.h for method setPartialWindow()
// showPartialUpdate() purposely uses values that are not multiples of 8 to test this

void showPartialUpdate()
{
  // some useful background
  helloWorld();
  // use asymmetric values for test
  uint16_t box_x = 10;
  uint16_t box_y = 15;
  uint16_t box_w = 70;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + box_h - 6;
  float value = 13.95;
  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  // show where the update box is
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
    //display.fillScreen(GxEPD_BLACK);
    display.displayWindow(box_x, box_y, box_w, box_h);
    delay(2000);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.displayWindow(box_x, box_y, box_w, box_h);
    delay(1000);
  }
  //return;
  // show updates in the update box
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    for (uint16_t i = 1; i <= 10; i += incr)
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
      display.setCursor(box_x, cursor_y);
      display.print(value * i, 2);
      display.displayWindow(box_x, box_y, box_w, box_h);
      delay(500);
    }
    delay(1000);
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.displayWindow(box_x, box_y, box_w, box_h);
    delay(1000);
  }
}


void drawBitmaps()
{
  display.setFullWindow();
#ifdef _GxBitmaps104x212_H_
  drawBitmaps104x212();
#endif
#ifdef _GxBitmaps128x250_H_
  drawBitmaps128x250();
#endif
#ifdef _GxBitmaps128x296_H_
  drawBitmaps128x296();
#endif
#ifdef _GxBitmaps176x264_H_
  drawBitmaps176x264();
#endif
#ifdef _GxBitmaps400x300_H_
  drawBitmaps400x300();
#endif
#ifdef _GxBitmaps640x384_H_
  drawBitmaps640x384();
#endif
#ifdef _WS_Bitmaps800x600_H_
  drawBitmaps800x600();
#endif
  // 3-color
#ifdef _GxBitmaps3c104x212_H_
  drawBitmaps3c104x212();
#endif
#ifdef _GxBitmaps3c128x296_H_
  drawBitmaps3c128x296();
#endif
#ifdef _GxBitmaps3c176x264_H_
  drawBitmaps3c176x264();
#endif
#ifdef _GxBitmaps3c400x300_H_
  drawBitmaps3c400x300();
#endif
  // show these after the specific bitmaps
#ifdef _GxBitmaps200x200_H_
  drawBitmaps200x200();
#endif
  // 3-color
#ifdef _GxBitmaps3c200x200_H_
  drawBitmaps3c200x200();
#endif
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    delay(10);
    Serial.println("setup");
    M5.begin(false,false,true);
    delay(100);
    M5.update();
    // if (M5.BtnMID.isPressed()) {
        display.init(115200);
        display.setRotation(0);
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.setTextSize(3);
        display.setFullWindow();
        // helloWorld();
        delay(1000);
    // }

    Wire.begin(25,26);

    if (airSensor.begin(Wire) == false) {
        Serial.println("Air sensor not detected. Please check wiring. Freezing...");
        while (1);
    }

    Serial.println("setup done");
    delay(10);
}

void loop() {
    // first update should be full refresh
    // helloWorld();
    // shutdown(10);
    // delay(1000);
    // // partial refresh mode can be used to full screen,
    // // effective if display panel hasFastPartialUpdate
    // helloFullScreenPartialMode();
    // delay(1000);
    // helloArduino();
    // delay(1000);
    // helloEpaper();
    // delay(1000);
    // //helloValue(123.9, 1);
    // //delay(1000);
    // showFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
    // delay(1000);
    // drawBitmaps();
    // if (display.epd2.hasPartialUpdate) {
        // showPartialUpdate();
        // delay(1000);
    // }  // else // on GDEW0154Z04 only full update available, doesn't look nice
    // //drawCornerTest();
    // //showBox(16, 16, 48, 32, false);
    // //showBox(16, 56, 48, 32, true);
    // display.powerOff();
    // deepSleepTest();


    // uint16_t box_x = 10;
    // uint16_t box_y = 15;
    // uint16_t box_w = 70;
    // uint16_t box_h = 20;
    // uint16_t cursor_y = box_y + box_h - 6;
    // float value = 13.95;
    // uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
    // display.setFont(&FreeMonoBold9pt7b);
    // display.setTextColor(GxEPD_BLACK);
    // display.setFullWindow();

    // for (uint16_t r = 0; r < 1; r++) {
    //     display.setRotation(r);
    //     for (uint16_t i = 1; i <= 10; i += incr) {
    //         display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    //         display.setCursor(box_x, cursor_y);
    //         display.print(value * i, 2);
    //         display.displayWindow(box_x, box_y, box_w, box_h);
    //         delay(500);
    //     }
    //     delay(1000);
    //     display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    //     display.displayWindow(box_x, box_y, box_w, box_h);
    //     delay(1000);
    // }
    // Serial.println("shutdown");
    // display.hibernate();
    // M5.shutdown(15);

    // while(1);

    if (airSensor.dataAvailable()) {
        Serial.print("co2(ppm):");
        Serial.print(airSensor.getCO2());
        helloValue (airSensor.getCO2(),0);

        Serial.print(" temp(C):");
        Serial.print(airSensor.getTemperature(), 1);

        Serial.print(" humidity(%):");
        Serial.print(airSensor.getHumidity(), 1);

        Serial.println();
    } else
        Serial.print(".");

    delay(1000);
}