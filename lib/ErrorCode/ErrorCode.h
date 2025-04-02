#ifndef ERROR_CODE_H
#define ERROR_CODE_H

#include <Arduino.h>

#define CODE_SUCCESS (0)

// NFC error codes
#define ERROR_PN532_INIT_FAILED         (100) // Initialization of the nfc module failed
#define ERROR_NFC_TAG_NOT_PRESENT       (101)
#define ERROR_INVALID_UID               (102)
#define ERROR_NO_NDEF_MESSAGE           (103)
#define ERROR_TOO_MANY_RECORDS          (104)
#define ERROR_NOT_ENOUGH_RECORDS        (105)
#define ERROR_INVALID_PAYLOAD           (106) // When the payload of the record is invalid
#define ERROR_INVALID_TNF               (107)
#define ERROR_COULD_NOT_WRITE_TAG       (108)

// Display error codes
#define ERROR_DISPLAY_INIT_FAILED       (200) // Initialization of the display failed

extern uint8_t errorCode; // Global variable to store the error code

#endif // ERROR_CODE_H