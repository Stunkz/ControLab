#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NfcAdapter.h>
#include <PN532.h>
#include <PN532_I2C.h>
#include <WiFi.h>
#include <Wire.h>

#include <Macro.h>

#define NFC_INTERFACE_I2C

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
  info("\nConnecting to network...\n");
  WiFi.begin(ssid, password);

  // Attente de la connexion Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    info(".");
  }

  info("Connected to WiFi! \n");
  info("IP address: %s\n", WiFi.localIP().toString().c_str());
}

void serverSetup() {
  // Démarrage du serveur TCP
  tcpServer.begin();
  tcpServer.setNoDelay(true);  // Désactiver le buffering pour réduire la latence
  info("Server started on port %d\n", serverPort);
}

void checkClientConnection() {
  if (!client || !client.connected()) {
    client = tcpServer.accept();  // Accepter un nouveau client
    if (client) {
      info("New client connected ! \n");
    }
  }
}

/*
==========================================================================
              Script écran OLED
==========================================================================
*/
void screenSetup() {
  info("Setting up oled screen...\n");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    error("Allocation failed\n");
    return;
  }
  
  info("Oled screen setted showing splash screen.\n");
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  
  delay(1000);
  display.clearDisplay();

  info("Showing CampusFab logo\n");
  for (int i = 0; i < 200; i++) {
    display.clearDisplay();

    display.drawBitmap(i%148,0,logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
    display.drawBitmap((i%148)-148,0,logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);

    display.display();
  }
  

  info("Oled screen setup finished ! \n");
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
  info("Cheking NDEF Reader Connection \n");

  nfc.begin();
}

// Check 
bool nfcTagChecker(NfcTag* tag) {

  if (!nfc.tagPresent()) {
    debug("NfcTag not found\n");
    return false;
  }

  *tag = nfc.read();

  info("NfcTag Found : %s %s\n", tag->getTagType().c_str(), tag->getUidString().c_str());
  return true;
}

bool getRecordPayload(NdefRecord record, byte* payload) {
  
  if (record.getTnf() != TNF_VALUE) {
    warn("Wrong TNF Value");
    return false;
  }
}

bool getNDEFMessage(byte* ndefMessage, NfcTag tag) {

  if (!tag.hasNdefMessage()) {
    warn("NfcTag don't have ndefmessage");
    return false;
  }

  

}

void checkNfcTag() {
  info("Checking NfcTag...\n");

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

  info("CONTROL LAB :((\n");

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
    Serial.print("Received: ");
    Serial.println(receivedMessage);

    // Répondre au client
    String reply = "Message reçu : " + receivedMessage;
    client.println(reply);
    Serial.print("Replied: ");
    Serial.println(reply);
  }

  // Permettre à l'utilisateur via le moniteur série d'envoyer un message au client
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');  // Lire les données entrées sur le moniteur série
    message.trim();  // Supprimer les espaces inutiles

    if (client && client.connected()) {
      client.println("Serveur : " + message);
      Serial.print("Sent to client: ");
      Serial.println(message);
    } else {
      Serial.println("No client connected.");
    }
  }
  */
}
