#include "NfcHandler.h"

NfcHandler nfcAntenna(&Wire);

NfcHandler::NfcHandler(TwoWire* wire) 
    : pn532_i2c(*wire), nfcShield(pn532_i2c), nfcAdapter(pn532_i2c) {}

bool NfcHandler::isNfcTagValid(NfcTag& tag) {
    if (tag.getUidString() == "") {
        log_w("No tag found");
        return false;
    }
    if (!tag.hasNdefMessage()) {
        log_w("No NDEF Message found on the tag");
        return false;
    }
    return true;
}

void NfcHandler::test() {
    if (!nfcAdapter.tagPresent()) {
        log_w("No tag present");
        return;
    }
    nfcShield.begin();
    uint32_t version = nfcShield.getFirmwareVersion();
    if (!version) {
        log_e("Didn't find PN53x board");
        return;
    }
    nfcShield.SAMConfig();
    log_d("Found chip PN532 v%.2X", version);

    log_v("Waiting for an NFC tag...");
    uint8_t uid[7];
    uint8_t uidLength = 0;
    if (nfcShield.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        log_d("Found an NFC tag with UID: ");
        String uidString = "";
        for (uint8_t i = 0; i < uidLength; i++) {
            uidString += String(uid[i], HEX) + " ";
        }
        log_d("UID: %s", uidString.c_str());
        String dataString = "";
        for (uint8_t page = 0; page < 128; page++) {
            uint8_t data[4] = {0};
            if (nfcShield.mifareultralight_ReadPage(page, data)) {
                String pageString = "";
                for (uint8_t i = 0; i < 4; i++) {
                    dataString += String(data[i], HEX) + " ";
                }
            } else {
                log_w("Failed to read page %d", page);
            }
        }
        log_d("Data: %s", dataString.c_str());
    } else {
        log_w("Failed to read NFC tag UID");
    }
    
}

bool NfcHandler::isNdefMessageValid(NdefMessage message) {
    if (message.getRecordCount() != 1) {
        log_w("Expected 1 NDEF record, found %d", message.getRecordCount());
        return false;
    }
    return true;
}

bool NfcHandler::isRecordValid(NdefRecord record) {
    if (record.getPayloadLength() > MAX_BYTES_MESSAGE) {
        log_w("Payload too long, expected less than %d bytes, found %d", MAX_BYTES_MESSAGE, record.getPayloadLength());
        return false;
    }
    if (record.getTnf() != EXPECTED_TNF_VALUE) {
        log_w("Wrong TNF value, expected %d, found %d", EXPECTED_TNF_VALUE, record.getTnf());
        return false;
    }
    return true;
}

bool NfcHandler::begin() {
    if (!nfcAdapter.begin()) {
        log_e("Error initializing NFC adapter");
        return false;
    }
    return true;
}

bool NfcHandler::getNfcTag(NfcTag& tag) {
    if (!nfcAdapter.tagPresent()) {
        return false;
    }
    tag = nfcAdapter.read();
    return true;
}

bool NfcHandler::getNdefMessage(NfcTag& tag, NdefMessage& message) {
    if (!isNfcTagValid(tag)) {
        return false;
    }
    message = tag.getNdefMessage();
    log_d("NDEF Message with %d records found", message.getRecordCount());

    return true;
}

bool NfcHandler::getPayload(NdefMessage& message, byte* payload) {
    if (!isNdefMessageValid(message)) {
      return false;
    }

    NdefRecord record = message.getRecord(0);

    if (!isRecordValid(record)) {
      return false;
    }
    
    record.getPayload(payload);

    return true;
}

bool NfcHandler::readNfcTag(byte* payload) {
    NfcTag tag;
    NdefMessage message;

    if (!getNfcTag(tag)) {
        return false;
    }

    if (!getNdefMessage(tag, message)) {
        return false;
    }

    if (!getPayload(message, lastPayload)) {
        return false;
    }

    if (payload) {
        memcpy(payload, lastPayload, MAX_BYTES_MESSAGE);
    }

    return true;
}

void NfcHandler::getLastPayload(byte* payload) {
    memcpy(payload, nfcAntenna.lastPayload, MAX_BYTES_MESSAGE);
}

String NfcHandler::getLastPayloadString() {
    String lastPayloadString = "";
    for (int i = 0; i < MAX_BYTES_MESSAGE; i++) {
        lastPayloadString += String(lastPayload[i], HEX) + " ";
    }
    return lastPayloadString;
}
