#ifndef NFC_HANDLER_H
#define NFC_HANDLER_H

#include <Arduino.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <esp32-hal-log.h>

#define MAX_BYTES_MESSAGE 16

// Define the expected TNF (Type Name Format) value for validation
constexpr uint8_t EXPECTED_TNF_VALUE = 0x01; // Example: 0x00 for NFC Well Known Type

class NfcHandler {
    private:
        PN532_I2C pn532_i2c;
        NfcAdapter nfcAdapter;
        byte lastPayload[MAX_BYTES_MESSAGE];

        bool isNfcTagValid(NfcTag& tag);
        bool isNdefMessageValid(NdefMessage message);
        bool isRecordValid(NdefRecord record);
        bool getNfcTag(NfcTag& tag);
        bool getNdefMessage(NfcTag& tag, NdefMessage& message);
        bool getPayload(NdefMessage& message, byte* payload);
        bool setRecord(NdefRecord& record, byte* payload);
        bool formatNfcMessage(NdefMessage& message, NdefRecord& record);
    
    public:
        NfcHandler(TwoWire* wire);

        bool begin();
        NdefMessage getNdefMessage();
        bool readNfcTag(byte* payload = nullptr);
        bool writeNfcTag(byte* payload);
        void getLastPayload(byte* payload);
        String getLastPayloadString();
};

extern NfcHandler nfcAntenna;

#endif // NFC_HANDLER_H