#include "DisplayHandler.h"

Display display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, SCREEN_ADDRESS);

// Constructeur
Display::Display(int width, int height, TwoWire* wire, uint8_t address) {
    screenWidth = width;
    screenHeight = height;
    screenAddress = address;
    oled = Adafruit_SSD1306(screenWidth, screenHeight, wire, OLED_RESET);
}


/**
 * @brief Initializes the OLED display.
 * 
 * @return true if the OLED display is successfully initialized, false otherwise.
 */
uint8_t Display::begin(void) {
    bool success;

    log_i("Initializing OLED display at address 0x%02X", screenAddress);

    // Initialize the OLED display with the specified address and settings
    // The OLED_RESET parameter is set to -1, indicating that the reset pin is not used.
    success = oled.begin(SSD1306_SWITCHCAPVCC, screenAddress);

    if (!success) {
        return ERROR_DISPLAY_INIT_FAILED;
    }

    oled.display();
    clear();

    return CODE_SUCCESS;
}

/**
 * @brief Clears the OLED display and optionally introduces a delay.
 * 
 * @param delayMs The delay in milliseconds to wait after clearing the display.
 */
void Display::clear(int delayMs) {

    oled.clearDisplay();
    oled.display();

    delay(delayMs);
}

/**
 * @brief Draws the CampusFab logo on the OLED display at the specified position.
 * 
 * @param x The x-coordinate where the logo will be drawn.
 * @param y The y-coordinate where the logo will be drawn.
 * @param delayMs The delay in milliseconds after displaying the logo.
 */
void Display::drawCampusFab(int x, int y, int delayMs) {

    oled.clearDisplay();
    oled.drawBitmap(x, y, logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
    oled.display();

    delay(delayMs);
}

void Display::text(const char* line1, const char* line2, const char* line3, int delayMs) {

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0,0);
    
    log_v("Showing text on display");

    if (line1 && line1[0] != '\0') {
        log_v("Line 1: %s", line1);
        oled.println(line1);
    }
    if (line2 && line2[0] != '\0') {
        log_v("Line 2: %s", line2);
        oled.println(line2);
    }
    if (line3 && line3[0] != '\0') {
        log_v("Line 3: %s", line3);
        oled.println(line3);
    }

    oled.display();
}