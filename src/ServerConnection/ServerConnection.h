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

        String charToString(const char* str, int len) {
            String result = "";
            for (int i = 0; i < len; i++) {
                result += str[i];
            }
            return result;
        }

    public:
        ServerConnection(void);

        uint8_t connect(String url);
        uint8_t checkConnection(void);
        uint8_t sendRequest(const char* request, int requestLen, const char* payload, int payloadLen, int* responseCode);
        String getResponse(void);
        String getErrorToString(int httpResponseCode);

};

#endif // SERVER_CONNECTION_H