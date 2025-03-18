#define NFC_INTERFACE_I2C

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NfcAdapter.h>
#include <PN532.h>
#include <PN532_I2C.h>
#include <WiFi.h>
#include <Wire.h>

#include <esp32-hal-log.h>

/*
==========================================================================
              Config wifi
==========================================================================
*/
// Indentifiants pour se connecter au routeur
const char *ssid = "controlLab";
const char *password = "jeromeray69@hotmail.fr";

const int serverPort = 69;

// Serveur TCP sur le port 69
WiFiServer tcpServer(serverPort);
// Client connecté au serveur
WiFiClient client;

extern "C" int lwip_hook_ip6_input(void *p) {
  return 1; // Retourne 1 pour indiquer que le paquet IPv6 est accepté
}

/*
==========================================================================
            Config écran OLED
==========================================================================
*/
#include <Logo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*
==========================================================================
        Config I2C
==========================================================================
*/
// Définir les nouvelles broches pour I2C
#define SDA_PIN 6
#define SCL_PIN 7

/*
==========================================================================
        Config lecteur NFC
==========================================================================
*/
#define TNF_VALUE 0x01
#define MAX_BYTES_MESSAGE 16

// Initialisation du PN532 via I2C
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

/*
==========================================================================
        Script wifi
==========================================================================
*/

/**
 * @brief Establishes a connection to a Wi-Fi network using the provided SSID and password.
 * 
 * This function attempts to connect to a Wi-Fi network and waits until the connection
 * is successfully established.
 * 
 * @note Ensure that the variables `ssid` and `password` are defined and contain
 *       the correct credentials for the Wi-Fi network before calling this function.
 * 
 * @warning This function blocks execution until the Wi-Fi connection is established.
 */
void setupWifiConnection() {
  log_i("Connecting to network...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  log_v("Waiting for connection...");
  }

  log_v("Connected to WiFi!");
  log_i("IP address: %s", WiFi.localIP().toString().c_str());
}

/**
 * @brief Initializes and starts the TCP server.
 * 
 * This function sets up the TCP server to begin listening for incoming 
 * connections. It also enables the TCP_NODELAY option to reduce latency.
 * 
 * @note Ensure that the `tcpServer` object and `serverPort` variable are 
 * properly initialized before calling this function.
 */
void setupTcpServer() {

  tcpServer.begin();
  tcpServer.setNoDelay(true);

  log_i("Server started on port %d", serverPort);
}

/**
 * @brief Checks the connection status of the client and accepts a new client
 *        connection if the current client is not connected.
 * 
 * This function verifies whether the current client is valid and connected.
 * If the client is either invalid or disconnected, it attempts to accept a
 * new client connection from the TCP server.
 */
void checkClientConnection() {
  if (!client || !client.connected()) {
  client = tcpServer.accept();
  if (client) {
    log_i("New client connected!");
  }
  }
}

/*
==========================================================================
        Script écran OLED
==========================================================================
*/

/**
 * @brief Draws the CampusFab logo on the display at the specified coordinates.
 * 
 * @param x The x-coordinate where the logo will be drawn.
 * @param y The y-coordinate where the logo will be drawn.
 */
void drawCampusFab(int x, int y) {
  display.clearDisplay();
  display.drawBitmap(x, y, logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
}

/**
 * @brief Clears the display screen and optionally introduces a delay.
 * 
 * @param delayMs The delay in milliseconds after clearing the screen. 
 *                Defaults to 0 if not specified.
 */
void clearScreen(int delayMs = 0) {
  display.clearDisplay();
  display.display();
  delay(delayMs);
}

/**
 * @brief Initializes and sets up the OLED screen.
 * 
 * This function initializes the OLED screen using the specified address and 
 * displays a splash screen followed by the CampusFab logo.
 * 
 * @note The function uses the SSD1306 library for OLED screen control.
 */
void setupScreen() {
  log_i("Setting up OLED screen...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    log_e("Allocation failed");
    return;
  }
  
  log_v("OLED screen set up, showing splash screen.");
  display.display();
  
  delay(1000);
  clearScreen(500);

  log_v("Showing CampusFab logo");
  drawCampusFab(0, 0);
  delay(2000);
  clearScreen();
  
  log_i("OLED screen setup finished!");
}

/*
==========================================================================
        Script lecteur NFC
==========================================================================
*/

/**
 * @brief Initializes the NFC card reader.
 * 
 */
void cardReaderSetup() {
  log_v("Checking NDEF Reader Connection");

  nfc.begin();
}

bool getPayload(byte* payload) {
  if (!nfc.tagPresent()) {
    log_i("No NfcTag found");
    return false;
  }
  NfcTag tag = nfc.read();

  if (!tag.hasNdefMessage()) {
    log_w("No NDEF Message found on the tag");
    return false;
  }

  NdefMessage message = tag.getNdefMessage();

  if (message.getRecordCount() != 1) {
    log_w("Expected 1 NDEF record, found %d", message.getRecordCount());
    return false;
  }

  NdefRecord record = message.getRecord(0);

  if (record.getPayloadLength() > MAX_BYTES_MESSAGE) {
    log_w("Payload too long, expected less than %d bytes, found %d", MAX_BYTES_MESSAGE, record.getPayloadLength());
    return false;
  }

  if (record.getTnf() != TNF_VALUE) {
    log_w("Wrong TNF value, expected %d, found %d", TNF_VALUE, record.getTnf());
    return false;
  }

  record.getPayload(payload);
  return true;
}

//TODO Make a real function
void wrongTag() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Wrong Tag!");
  display.display();
}

void checkNfcTag() {
  log_v("Checking NfcTag...");

  byte payload[MAX_BYTES_MESSAGE];
  if (!getPayload(payload)) {
    wrongTag();
    return;
  }
  log_d("Payload: %s", payload);

  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("NFC Tag Found!");
  display.display();
  
}

/*
==========================================================================
        Script principal
==========================================================================
*/

void setup() {
  Serial.begin(115200);

  log_i("CONTROL LAB :((");

  // Configurer les broches I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  setupScreen();
  
  setupWifiConnection();
  setupTcpServer();
  

  cardReaderSetup();
}

void loop() {
  checkClientConnection();
  checkNfcTag();
  
  delay(1000);
}
