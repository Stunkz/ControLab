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
// Setup la connection au wifi configuré dans la partie config du script.
// La fonction Ne s'arrête pas tant qu'elle n'est pas connecté au wifi.
void wifiConnection() {
  // Connexion au Wi-Fi
  log_i("Connecting to network...");
  WiFi.begin(ssid, password);

  // Attente de la connexion Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  log_i(".");
  }

  log_v("Connected to WiFi!");
  log_v("IP address: %s", WiFi.localIP().toString().c_str());
}

void serverSetup() {
  // Démarrage du serveur TCP
  tcpServer.begin();
  tcpServer.setNoDelay(true);  // Désactiver le buffering pour réduire la latence
  log_i("Server started on port %d", serverPort);
}

void checkClientConnection() {
  if (!client || !client.connected()) {
  client = tcpServer.accept();  // Accepter un nouveau client
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
void screenSetup() {
  log_i("Setting up OLED screen...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    log_e("Allocation failed");
    return;
  }
  
  log_i("OLED screen set up, showing splash screen.");
  display.display();
  
  delay(1000);
  display.clearDisplay();

  log_i("Showing CampusFab logo");
  for (int i = 0; i < 200; i++) {
    display.clearDisplay();

    display.drawBitmap(i%148,0,logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
    display.drawBitmap((i%148)-148,0,logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);

    display.display();
  }
  
  log_i("OLED screen setup finished!");
}

void drawLogoFun() {}

/*
==========================================================================
        Script lecteur NFC
==========================================================================
*/
// Initialize the NDEF Reader board and return the firmware version
// If no module is found, the program is stopped
void cardReaderSetup() {
  log_v("Checking NDEF Reader Connection");

  nfc.begin();
}

// TODO: assert failed: heap_caps_free heap_caps_base.c:63 (heap != NULL && "free() target pointer is outside heap areas")
void checkNfcTag() {
  log_v("Checking NfcTag...");

  NfcTag tag;
  if (!nfc.tagPresent()) {
    log_i("No NfcTag found");
    return;
  }

  tag = nfc.read();

  // TODO: Regler l'affichage du tag qui fait crash le programme
  //log_d("NfcTag Found: %s %s", tag.getTagType().c_str(), tag.getUidString().c_str());

  if (!tag.hasNdefMessage()) {
    log_w("No NDEF Message found on the tag");
    return;
  }

  NdefMessage message = tag.getNdefMessage();
  byte payload[MAX_BYTES_MESSAGE] = {0};
  log_d("NDEF Message found with %d records", message.getRecordCount());

  if (message.getRecordCount() != 1) {
    log_w("Expected 1 NDEF record, found %d", message.getRecordCount());
    return;
  }

  NdefRecord record = message.getRecord(0);

  if (record.getPayloadLength() > MAX_BYTES_MESSAGE) {
    log_w("Payload too long, expected less than %d bytes, found %d", MAX_BYTES_MESSAGE, record.getPayloadLength());
    return;
  }

  record.getPayload(payload);
  log_d("Payload: %s", payload);

  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("NFC Tag Found!");
  display.println("UID: " + tag.getUidString());
  display.println("Type: " + tag.getTagType());
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

  screenSetup();
  /*
  wifiConnection();
  serverSetup();
  */

  cardReaderSetup();
}

void loop() {
  checkClientConnection();
  checkNfcTag();
  
  delay(1000);
}
