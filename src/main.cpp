/*
Fonctionnalités:
Lorsqu'on scan un tag on récupere le payload. On vérifie ensuite avec la raspberry pi ou le serveur si c'est un identifiant valide.
Coté serveur on recrée un identifiant et on le donne à l'esp32 pour le reécrire sur le tag.

Il faut aussi un mode pour écrire simplement sur le tag avec soit un boutton soit un mode.
*/

#define NFC_INTERFACE_I2C

#include <DisplayHandler.h>
#include <NfcHandler.h>
#include <ServerConnection.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <Wire.h>

#include <esp32-hal-log.h>

#include <ErrorCode.h>

/*
==========================================================================
              Config wifi
==========================================================================
*/
// Indentifiants pour se connecter au routeur
const char *ssid = "controlLab";
const char *password = "jeromeray69@hotmail.fr";

const String host = "http://192.168.0.105:8000";

extern "C" int lwip_hook_ip6_input(void *p) {
  return 1; // Retourne 1 pour indiquer que le paquet IPv6 est accepté
}

ServerConnection server = ServerConnection();

/*
==========================================================================
        Config I2C
==========================================================================
*/
// Définir les nouvelles broches pour I2C
#define SDA_PIN 6
#define SCL_PIN 7

#define I2C_CLOCK 100000 // 100kHz

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

void setupServerConnection() {
  log_i("Connecting to server...");
  do {
    errorCode = server.connect(host);

    if (errorCode != CODE_SUCCESS) {

      display.text("Server", "Error", "", 0);
      log_e("Error connecting to server: %d", errorCode);

      delay(1000);
    } else {

      display.text("Server", "Ready", "", 0);
      log_v("Server connected successfully.");

    }
  } while (errorCode != CODE_SUCCESS);
}


bool sendCardID(const char* cardID) {
  if (checkConnection() > 0) {
    log_e("Unable to connect to server or WiFi.");
    return false;
  }

  log_v("Sending card ID to server...");
  String payload = "101";
  for (int i = 0; i < PAYLOAD_SIZE; i++) {
    payload += String(cardID[i]);
  }
  log_d("Payload: %s", payload.c_str());

  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    String response = http.getString();
    log_d("Response: %s", response.c_str());
    String request = "";
    for (int i = 0; i < 3; i++) {
      request += response[i];
    }
    if (strcmp(request.c_str(), "001") == 0) {
      log_v("Valid ID");
      String newID = "";
      for (int i = 0; i < PAYLOAD_SIZE; i++) {
        newID += String(response[i+3]);
      }
      nfcAntenna.writeNfcTag((byte*)newID.c_str());
    } else if (strcmp(request.c_str(), "002") == 0) {
      log_v("Invalid ID");
    } else {
      log_v("Invalid response format, got %s", request.c_str());
    }
    
  } else {
    log_e("Error on sending POST: %s", http.errorToString(httpResponseCode).c_str());
    return false;
  }
  return true;
}

/*
==========================================================================
        Script lecteur NFC
==========================================================================
*/

//TODO Make a real function
void wrongTag() {

  display.text("Wrong Tag", "", "", 1000);

}

void checkNfcTag() {
  byte payload[PAYLOAD_SIZE] = {0};
  char cardID[PAYLOAD_SIZE] = {0};

  log_v("Checking NfcTag...");
  display.text("Checking Tag...", "", "", 0);

  // Reading the NFC tag
  // If no tag is present, the function will return an error code
  // If a tag is present, it will read the tag and return the code success
  // If the tag is not valid, it will return an error code
  errorCode = nfcAntenna.readNfcTag();

  if (errorCode == ERROR_NFC_TAG_NOT_PRESENT) {

    log_v("No NFC tag present.");
    return;
  } else if (errorCode != CODE_SUCCESS) {

    log_e("Error reading NFC tag: %d", errorCode);
    wrongTag();

    return;
  }
  log_v("Tag found!");

  // Getting the last payload that has been read
  errorCode = nfcAntenna.getLastPayload(payload);

  if (errorCode != CODE_SUCCESS) {

    log_e("Error getting last payload: %d", errorCode);
    wrongTag();

    return;
  }

  memcpy(cardID, payload, PAYLOAD_SIZE);
  log_d("Card ID: %s", nfcAntenna.getLastPayloadString().c_str());
  
  // Sending card to the server to verify its validity
  // and get a new ID if it's valid
  // sendCardID(cardID);
}

void setup() {
  Serial.begin(115200);

  log_i("CONTROL LAB :((");

  // Config I2C
  // Set the I2C pins to the new ones
  Wire.begin(SDA_PIN, SCL_PIN);
  // Set the I2C frequency to 100kHz
  Wire.setClock(I2C_CLOCK);

  // Config display
  errorCode = display.begin();
  if (errorCode != CODE_SUCCESS) {

    log_e("Error initializing display: %d", errorCode);
    return;
  }

  display.drawCampusFab(0, 0, 2000);
  display.clear();

  
  display.text("Waiting for", "Connection...", "", 0);
  setupWifiConnection();

  setupServerConnection();
  
  display.text("Waiting for", "NFC Antenna...", "", 0);

  // Start the NFC antenna
  do {

    errorCode = nfcAntenna.begin();

    if (errorCode != CODE_SUCCESS) {

      display.text("NFC Antenna", "Error", "", 0);
      log_e("Error initializing NFC antenna: %d", errorCode);

      delay(1000);
    } else {

      display.text("NFC Antenna", "Ready", "", 0);
      log_v("NFC antenna initialized successfully.");

    }

  } while(errorCode != CODE_SUCCESS);
}
 
void loop() {
  // nfcAntenna.writeNfcTag((byte*)"1234567891");
  checkNfcTag();
  
  delay(1000);
}
