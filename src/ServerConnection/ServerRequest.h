#ifndef SERVER_REQUEST_H
#define SERVER_REQUEST_H

#include <Arduino.h>

#define REQUEST_SIZE 3 // Size of the request string

// Server response
constexpr const char* VALID_CARD_ID = "001";          // 001 = The card is in the database and is valid
constexpr const char* INVALID_CARD_ID = "002";        // 002 = The card is either not in the database or is invalid
constexpr const char* INVALID_FORMAT = "003";         // 003 = The card ID is not in the correct format
constexpr const char* INVALID_REQUEST = "400";        // 400 = The request is invalid

// Server request
constexpr const char* CARD_ID_VERIFICATION = "101";   // 101 = Card ID verification

#endif // SERVER_REQUEST_H
