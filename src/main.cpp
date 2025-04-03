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
#include <Audio.h>

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


uint8_t sendCardID(const char* cardID, String& newID) {
  int httpResponseCode = 0;
  String response, request = "";
  
  errorCode = server.checkConnection();
  if (errorCode != CODE_SUCCESS) {

    log_e("Error checking server connection: %d", errorCode);

    return errorCode;
  }

  log_v("Sending card ID to server...");
  
  server.sendRequest(CARD_ID_VERIFICATION, 3, cardID, PAYLOAD_SIZE, &httpResponseCode);

  if (httpResponseCode > 0) {

    response = server.getResponse();
    log_d("Response: %s", response.c_str());

    for (int i = 0; i < 3; i++) {
      request += response[i];
    }

    if (strcmp(request.c_str(), "001") == 0) {

      log_v("Valid ID");

      for (int i = 0; i < PAYLOAD_SIZE; i++) {
        newID += String(response[i+3]);
      }

    } else if (strcmp(request.c_str(), "002") == 0) {

      log_v("Invalid ID");

      return ERROR_INVALID_CARD_ID;
    } else {

      log_v("Invalid response format, got %s", request.c_str());

      return ERROR_INVALID_SERVER_RESPONSE;
    }
    
  } else {
    log_e("Error on sending POST: %s", server.getErrorToString(httpResponseCode).c_str());
    return ERROR_INVALID_REQUEST;
  }
  return CODE_SUCCESS;
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

void goodTag() {

  display.text("Valid", "", "", 1000);
}

void checkNfcTag() {
  byte payload[PAYLOAD_SIZE] = {0};
  char cardID[PAYLOAD_SIZE] = {0};
  String newID = "";

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
  errorCode = sendCardID(cardID, newID);
  if (errorCode != CODE_SUCCESS) {
    log_e("Error sending card ID: %d", errorCode);
    wrongTag();

    return;
  }

  nfcAntenna.writeNfcTag((byte*)newID.c_str());
}

#define I2S_DOUT      18
#define I2S_BCLK      8
#define I2S_LRC       9

Audio audio;

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

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21);
//  setupServerConnection();
  
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
  //checkNfcTag();
  audio.loop();
}
