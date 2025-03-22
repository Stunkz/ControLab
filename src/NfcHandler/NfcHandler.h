#ifndef NFC_HANDLER_H
#define NFC_HANDLER_H

#include <Arduino.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <NfcAdapter.h>
#include <esp32-hal-log.h>

#define MAX_BYTES_MESSAGE 16
#define TNF_VALUE 0x01 // Define the expected TNF (Type Name Format) value for validation

// Define the expected TNF (Type Name Format) value for validation
constexpr uint8_t EXPECTED_TNF_VALUE = 0x01; // Example: 0x01 for NFC Well Known Type

class NfcHandler {
    private:
        PN532_I2C pn532_i2c;
        NfcAdapter nfcAdapter;
        byte lastPayload[MAX_BYTES_MESSAGE];

        bool isNfcTagValid(NfcTag& tag);
        bool isNdefMessageValid(NdefMessage message);
        bool isRecordValid(NdefRecord record);
    
    public:
        NfcHandler(TwoWire* wire);

        bool begin();
        NdefMessage getNdefMessage();
        bool getPayload(byte* payload = nullptr);
        void getLastPayload(byte* payload);
};

extern NfcHandler nfcAntenna;

#endif // NFC_HANDLER_H