/*
Fonctionnalités:
Lorsqu'on scan un tag on récupere le payload. On vérifie ensuite avec la raspberry pi ou le serveur si c'est un identifiant valide.
Coté serveur on recrée un identifiant et on le donne à l'esp32 pour le reécrire sur le tag.

Il faut aussi un mode pour écrire simplement sur le tag avec soit un boutton soit un mode.
*/

#define NFC_INTERFACE_I2C

#include <DisplayHandler.h>
#include <NfcHandler.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>

#include <esp32-hal-log.h>

/*
==========================================================================
              Config wifi
==========================================================================
*/
// Indentifiants pour se connecter au routeur
const char *ssid = "controlLab";
const char *password = "jeromeray69@hotmail.fr";

const int port = 8080;
const char *host = "192.168.1.100:8000";

WiFiClient espClient;

extern "C" int lwip_hook_ip6_input(void *p) {
  return 1; // Retourne 1 pour indiquer que le paquet IPv6 est accepté
}

HTTPClient http;

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

void setupServerHTTP(const char* server) {
  log_i("Connecting to server...");
  while (!http.begin(espClient, server)) {
    delay(1000);
    log_v("Waiting for server connection...");
  }
  log_v("Connected to server!");
  http.addHeader("Content-Type", "text/plain");
}

uint8_t checkServerConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    log_e("WiFi disconnected, reconnecting...");
    return 1;
  }

  if (!http.connected()) {
    log_e("HTTP client disconnected, reconnecting...");
    return 2;
  }
  return 0;
}

bool sendCardID(const char* cardID) {
  if (checkServerConnection() > 0) {
    log_e("Error: Unable to connect to server or WiFi.");
    return false;
  }

  log_v("Sending card ID to server...");
  String payload = "001";
  for (int i = 0; i < MAX_BYTES_MESSAGE; i++) {
    payload += String(cardID[i]);
  }
  log_d("Payload: %s", payload.c_str());

  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    String response = http.getString();
    log_d("Response: %s", response.c_str());
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
  display.text("Wrong Tag", "", "", 0);
  log_w("Wrong Tag");
}

void checkNfcTag() {
  log_v("Checking NfcTag...");
  display.text("Checking Tag...", "", "", 0);
  
  if (!nfcAntenna.readNfcTag()) {
    wrongTag();
    return;
  }

  log_v("Tag found!");
  display.text("Tag found!", "", "", 0);
  byte payload[MAX_BYTES_MESSAGE];

  nfcAntenna.getLastPayload(payload);
  char cardID[MAX_BYTES_MESSAGE] = {0};

  for (int i = 0; i < MAX_BYTES_MESSAGE; i++) {
    cardID[i] = payload[i];
  }

  log_d("Card ID: %s", nfcAntenna.getLastPayloadString().c_str());
  
  sendCardID(cardID);
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

  display.begin();
  display.drawCampusFab(0, 0, 2000);
  display.clear();

  
  display.text("Waiting for", "Connection...", "", 0);
  setupWifiConnection();

  setupServerHTTP(host);
  
  display.text("Waiting for", "NFC Antenna...", "", 0);
  while (!nfcAntenna.begin()) {
    delay(1000);
  }
}

void loop() {
  nfcAntenna.writeNfcTag((byte*)"");
  checkNfcTag();
  
  delay(1000);
}
