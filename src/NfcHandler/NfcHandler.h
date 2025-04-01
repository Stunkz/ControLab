#ifndef NFC_HANDLER_H
#define NFC_HANDLER_H

#include <Arduino.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <esp32-hal-log.h>

#include <ErrorCode.h>

/*
Card are made of 10 bytes like "1234567890"
There is only 1 record in the NDEF message with a payload of 10 bytes
The TNF is set to 0x01 (NFC Forum well-known type)
*/
#define PAYLOAD_SIZE 10
#define NUM_RECORDS 1

// Define the expected TNF (Type Name Format) value for validation
#define EXPECTED_TNF_VALUE 0x01

class NfcHandler {
    private:
        PN532_I2C   pn532_i2c;
        NfcAdapter  nfcAdapter;
        byte        lastPayload[PAYLOAD_SIZE];



        uint8_t formatNfcMessage(NdefMessage& message, NdefRecord& record);

        uint8_t getNfcTag(NfcTag& tag);
        uint8_t getNdefMessage(NfcTag& tag, NdefMessage& message);
        uint8_t getPayload(NdefMessage& message, byte* payload);

        uint8_t isNfcTagValid(NfcTag& tag);
        uint8_t isNdefMessageValid(NdefMessage message);
        uint8_t isRecordValid(NdefRecord record);

        uint8_t setRecord(NdefRecord& record, byte* payload);
    
    public:
        NfcHandler(TwoWire* wire);



        uint8_t     begin();
        void        getLastPayload(byte* payload);
        String      getLastPayloadString();
        uint8_t     readNfcTag(byte* payload = nullptr);
        uint8_t     writeNfcTag(byte* payload);
        
};

extern NfcHandler nfcAntenna;

#endif // NFC_HANDLER_H