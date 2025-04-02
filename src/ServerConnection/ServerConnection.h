#ifndef SERVER_CONNECTION_H
#define SERVER_CONNECTION_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ErrorCode.h>
#include <ServerRequest.h>
#include <NfcHandler.h>
#include <WiFi.h>

class ServerConnection {

    private:
        HTTPClient http;

    public:
        ServerConnection(void);

        uint8_t connect(String url);
        uint8_t checkConnection(void);
        uint8_t sendRequest(const char* request, const char* payload);

};

#endif // SERVER_CONNECTION_H