#ifndef NFC_HANDLER_H
#define NFC_HANDLER_H

#include <Arduino.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <NfcAdapter.h>
#include <esp32-hal-log.h>

#define TNF_VALUE 0x01
#define MAX_BYTES_MESSAGE 16

class NfcHandler {
    private:
        PN532_I2C pn532_i2c;
        NfcAdapter nfcAdapter;
        byte lastPayload[MAX_BYTES_MESSAGE];

        bool isNfcTagValid(NfcTag tag);
        bool isNdefMessageValid(NdefMessage message);
        bool isRecordValid(NdefRecord record);
    
    public:
        NfcHandler(TwoWire* wire);

        bool begin();
        bool getPayload(byte* payload = nullptr);
        void getLastPayload(byte* payload);
};

extern NfcHandler nfcAntenna;

#endif // NFC_HANDLER_H