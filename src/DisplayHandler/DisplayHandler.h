#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <esp32-hal-log.h>
#include "Logo.h"
#include <ErrorCode.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

class Display {
    private:
        Adafruit_SSD1306 oled;
        int screenWidth;
        int screenHeight;
        uint8_t screenAddress;
    
    public:
        Display(int width, int height, TwoWire* wire, uint8_t address);
    
        uint8_t begin(void);
        void clear(int delayMs = 0);
        void drawCampusFab(int x, int y, int delayMs = 0);
        void text(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr, int delayMs = 0);

};

extern Display display;

#endif // DISPLAU_HANDLER_H