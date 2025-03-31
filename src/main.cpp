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

#include <esp32-hal-log.h>

/*
==========================================================================
              Config wifi
==========================================================================
*/
// Indentifiants pour se connecter au routeur
const char *ssid = "controlLab";
const char *password = "jeromeray69@hotmail.fr";

const int port = 9966;
const char *host = "192.168.1.100";

WiFiClient espClient;

extern "C" int lwip_hook_ip6_input(void *p) {
  return 1; // Retourne 1 pour indiquer que le paquet IPv6 est accepté
}

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

bool connectToServer(const char *host, int port) {
  log_i("Connecting to server %s:%d", host, port);
  if (!espClient.connect(host, port)) {
    log_e("Connection failed!");
    return false;
  }

  log_v("Connected to server!");
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
  /*
  if (!nfcAntenna.readNfcTag()) {
    wrongTag();
    return;
  }

  log_v("Tag found!");
  display.text("Tag found!", "", "", 0);
  log_d("Payload: %s", nfcAntenna.getLastPayloadString().c_str());
  */

  nfcAntenna.test();
  
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

  /*
  display.text("Waiting for", "Connection...", "", 0);
  setupWifiConnection();

  while (!connectToServer(host, port)) {
    delay(1000);
  }
  */
  
  display.text("Waiting for", "NFC Antenna...", "", 0);
  while (!nfcAntenna.begin()) {
    delay(1000);
  }
}

void loop() {
  checkNfcTag();
  
  delay(1000);
}
