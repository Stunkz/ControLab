#include "ServerConnection.h"

ServerConnection::ServerConnection() : http(HTTPClient()) {
}

uint8_t ServerConnection::connect(String url) {
    bool success;

    success = http.begin(url);
    if (!success) {
        return ERROR_SERVER_CONNECTION_FAILED;
    }

    http.addHeader("Content-Type", "text/plain");

    errorCode = checkConnection();
    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    return CODE_SUCCESS;
}

uint8_t ServerConnection::checkConnection() {
    int httpResponseCode;

    // Check wifi connection
    if (WiFi.status() != WL_CONNECTED) {

        log_e("Wifi disconnected.");
        return ERROR_WIFI_DISCONNECTED;
    }

    // Check if the server responds
    httpResponseCode = http.GET();
    if (httpResponseCode != 200) {

        log_e("Server disconnected.");
        return ERROR_SERVER_DISCONNECTED;
    }

    return CODE_SUCCESS;
}

uint8_t ServerConnection::sendRequest(const char* request, int requestLen, const char* payload, int payloadLen, int* responseCode) {
    String requestString, payloadString, fullRequest;

    if (request == nullptr) {
        return ERROR_INVALID_REQUEST;
    }

    if (requestLen != REQUEST_SIZE) {
        return ERROR_INVALID_REQUEST;
    }

    requestString = charToString(request, requestLen);
    

    if (payload == nullptr) {
        return ERROR_INVALID_SERVER_PAYLOAD;
    }

    if (payloadLen != PAYLOAD_SIZE) {
        return ERROR_INVALID_SERVER_PAYLOAD;
    }

    payloadString = charToString(payload, payloadLen);
    

    fullRequest = requestString + payloadString;
    log_d("Full request: %s", fullRequest.c_str());

    *responseCode = http.POST(fullRequest);
    return CODE_SUCCESS;
}

String ServerConnection::getResponse() {
    return http.getString();
}

String ServerConnection::getErrorToString(int httpResponseCode) {
    return http.errorToString(httpResponseCode).c_str();
}