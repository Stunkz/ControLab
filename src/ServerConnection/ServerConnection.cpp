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

uint8_t ServerConnection::sendRequest(const char* request, const char* payload) {
    String requestString, payloadString;

    if (request == nullptr) {
        return ERROR_INVALID_REQUEST;
    }

    requestString = String(request);
    if (requestString.length() != REQUEST_SIZE) {
        return ERROR_INVALID_REQUEST;
    }

    if (payload == nullptr) {
        return ERROR_INVALID_SERVER_PAYLOAD;
    }

    payloadString = String(payload);
    if (payloadString.length() != PAYLOAD_SIZE) {
        return ERROR_INVALID_SERVER_PAYLOAD;
    }

    String fullRequest = requestString + payloadString;
    log_d("Full request: %s", fullRequest.c_str());

    int httpResponseCode = http.POST(fullRequest);

}