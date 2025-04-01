#ifndef ERROR_CODE_H
#define ERROR_CODE_H

#define CODE_SUCCESS (0)

// NFC error codes
#define ERROR_PN532_INIT_FAILED         (100) // Initialization of the nfc module failed
#define ERROR_NFC_TAG_NOT_PRESENT       (101)
#define ERROR_INVALID_UID               (102)
#define ERROR_NO_NDEF_MESSAGE           (103)
#define ERROR_TOO_MANY_RECORDS          (104)
#define ERROR_NOT_ENOUGH_RECORDS        (105)


#endif // ERROR_CODE_H