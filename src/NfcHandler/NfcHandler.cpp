#include "NfcHandler.h"

NfcHandler nfcAntenna(&Wire);

NfcHandler::NfcHandler(TwoWire* wire) 
    : pn532_i2c(*wire), nfcAdapter(pn532_i2c) {}

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
    if (record.getTnf() != TNF_VALUE) {
        log_w("Wrong TNF value, expected %d, found %d", TNF_VALUE, record.getTnf());
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

void NfcHandler::getNfcTag(NfcTag& tag) {
    if (!nfcAdapter.tagPresent()) {
        return;
    }
    tag = nfcAdapter.read();
}

bool NfcHandler::getPayload(byte* payload) {
    NfcTag tag;
    getNfcTag(tag);
  
    if (!isNfcTagValid(tag)) {
      return false;
    }
  
    NdefMessage message = tag.getNdefMessage();
  
    if (!isNdefMessageValid(message)) {
      return false;
    }
  
    NdefRecord record = message.getRecord(0);
  
    if (!isRecordValid(record)) {
      return false;
    }
    
    record.getPayload(lastPayload);
    if (payload) {
        memcpy(payload, lastPayload, MAX_BYTES_MESSAGE);
    }
    return true;
}

void NfcHandler::getLastPayload(byte* payload) {
    memcpy(payload, nfcAntenna.lastPayload, MAX_BYTES_MESSAGE);
}


