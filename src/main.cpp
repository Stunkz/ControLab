#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NfcAdapter.h>
#include <PN532.h>
#include <PN532_I2C.h>
#include <WiFi.h>
#include <Wire.h>

#define NFC_INTERFACE_I2C
#define DEBUG

extern "C" int lwip_hook_ip6_input(void *p) {
    return 1; // Retourne 1 pour indiquer que le paquet IPv6 est accepté
}

/*
==========================================================================
                        Partie config wifi
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


/*
==========================================================================
                          Config écran OLED
==========================================================================
*/
#include "logo.h"

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
                              Macro
==========================================================================
*/

String formatTime(unsigned long milliseconds) {
    unsigned long seconds = milliseconds / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;

    milliseconds %= 1000;
    seconds %= 60;
    minutes %= 60;
    hours %= 24;

    char buffer[20];
    sprintf(buffer, "%02lu:%02lu:%02lu:%04lu", hours, minutes, seconds, milliseconds);
    return String(buffer);
}

template <typename T>
void printRec(const T& arg) {
  Serial.print(arg);
}

template <typename First, typename... Rest>
void printRec(const First& first, const Rest&... rest) {
  Serial.print(first);
  printRec(rest...);
}

template <typename... Args>
void print(const Args&... args) {
  Serial.print(formatTime(millis())); Serial.print(" ");
  printRec(args...);
}


/*
==========================================================================
                            Script wifi
==========================================================================
*/
// Setup la connection au wifi configuré dans la partie config du script.
// La fonction Ne s'arrête pas tant qu'elle n'est pas connecté au wifi.
void wifiConnection() {
  // Connexion au Wi-Fi
  print("\n[WiFi] Connecting to network...\n");
  WiFi.begin(ssid, password);

  // Attente de la connexion Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    print(".");
  }

  print("\n[WiFi] Connected to WiFi! \n");
  print("[WiFi] IP address: ", WiFi.localIP(), "\n");
}

void serverSetup() {
  // Démarrage du serveur TCP
  tcpServer.begin();
  tcpServer.setNoDelay(true);  // Désactiver le buffering pour réduire la latence
  print("[TCP] Server started on port ", serverPort, "\n");
}

void checkClientConnection() {
  if (!client || !client.connected()) {
    client = tcpServer.available();  // Accepter un nouveau client
    if (client) {
      print("[TCP] New client connected ! \n");
    }
  }
}

/*
==========================================================================
                          Script écran OLED
==========================================================================
*/
void screenSetup() {
  print("[SSD1306] Setting up oled screen...\n");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    print("[SSD1306] Allocation failed\n");
    return;
  }
  
  print("[SSD1306] Oled screen setted showing splash screen.\n");
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  
  delay(1000);
  display.clearDisplay();

  print("[SSD1306] Showing CampusFab logo\n");
  for (int i = 0; i < 200; i++) {
    display.clearDisplay();

    display.drawBitmap(i%148,0,logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
    display.drawBitmap((i%148)-148,0,logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);

    display.display();
  }
  

  print("[SSD1306] Oled screen setup finished ! \n");
}

void drawLogoFun() {

  
  
}

/*
==========================================================================
                          Script lecteur NFC
==========================================================================
*/
// Initialize the NDEF Reader board and return the firmware version
// If no module is found, the program is stopped
void cardReaderSetup() {
  print("[NFC] Cheking NDEF Reader Connection \n");

  nfc.begin();
}

// Check 
bool nfcTagChecker(NfcTag* tag) {

  if (!nfc.tagPresent()) {
    #ifdef DEBUG
    print("[NFC] NfcTag not found\n");
    #endif
    return false;
  }

  *tag = nfc.read();

  print("[NFC] NfcTag Found : ", tag->getTagType(), " ", tag->getUidString(), "\n");
  return true;
}

bool getRecordPayload(NdefRecord record, byte* payload) {
  
  if (record.getTnf() != TNF_VALUE) {
    #ifdef DEBUG
    print("[NFC] Wrong TNF Value");
    #endif
    return false;
  }
}

bool getNDEFMessage(byte* ndefMessage, NfcTag tag) {

  if (!tag.hasNdefMessage()) {
    print("[NFC] NfcTag don't have ndefmessage");
    return false;
  }

  

}

void checkNfcTag() {
  print("[NFC] Checking NfcTag...\n");

  NfcTag tag;
  if (!nfcTagChecker(&tag)) {
    return;
  }

}

/*
==========================================================================
                          Script principal
==========================================================================
*/

void setup() {
  Serial.begin(115200);

  print("CONTROL LAB :((\n");

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

/*
  // Vérifier si le client a envoyé des données
  if (client && client.connected() && client.available()) {
    String receivedMessage = client.readStringUntil('\n');  // Lire les données envoyées par le client
    receivedMessage.trim();  // Supprimer les espaces inutiles
    Serial.print("[TCP] Received: ");
    Serial.println(receivedMessage);

    // Répondre au client
    String reply = "Message reçu : " + receivedMessage;
    client.println(reply);
    Serial.print("[TCP] Replied: ");
    Serial.println(reply);
  }

  // Permettre à l'utilisateur via le moniteur série d'envoyer un message au client
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');  // Lire les données entrées sur le moniteur série
    message.trim();  // Supprimer les espaces inutiles

    if (client && client.connected()) {
      client.println("Serveur : " + message);
      Serial.print("[TCP] Sent to client: ");
      Serial.println(message);
    } else {
      Serial.println("[TCP] No client connected.");
    }
  }
  */
}
