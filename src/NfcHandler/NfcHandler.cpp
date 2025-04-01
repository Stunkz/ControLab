#include "NfcHandler.h"

/*
===================================================================================================================
                                                Private Methods
====================================================================================================================
*/

NfcHandler::NfcHandler(TwoWire* wire) 
    : pn532_i2c(*wire), nfcAdapter(pn532_i2c) {}

uint8_t NfcHandler::formatNfcMessage(NdefMessage& message, NdefRecord& record) {
    if (!isRecordValid(record)) {
        return false;
    }

    message.addRecord(record);
    return true;
}

uint8_t NfcHandler::getNfcTag(NfcTag& tag) {
    if (!nfcAdapter.tagPresent()) {
        return ERROR_NFC_TAG_NOT_PRESENT;
    }
    tag = nfcAdapter.read();
    return CODE_SUCCESS;
}

uint8_t NfcHandler::getNdefMessage(NfcTag& tag, NdefMessage& message) {
    if (!isNfcTagValid(tag)) {
        return false;
    }
    message = tag.getNdefMessage();
    log_d("NDEF Message with %d records found", message.getRecordCount());

    return true;
}

uint8_t NfcHandler::getPayload(NdefMessage& message, byte* payload) {
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

uint8_t NfcHandler::isNfcTagValid(NfcTag& tag) {
    if (tag.getUidString() == "") {
        return ERROR_INVALID_UID;
    }
    if (!tag.hasNdefMessage()) {
        return ERROR_NO_NDEF_MESSAGE;
    }
    return CODE_SUCCESS;
}

uint8_t NfcHandler::isNdefMessageValid(NdefMessage message) {
    int recordCount = message.getRecordCount();

    if (recordCount < NUM_RECORDS) {
        return ERROR_NOT_ENOUGH_RECORDS;
    }
    
    if (recordCount > NUM_RECORDS) {
        return ERROR_TOO_MANY_RECORDS;
    }
    return CODE_SUCCESS;
}

uint8_t NfcHandler::isRecordValid(NdefRecord record) {
    if (record.getPayloadLength() > PAYLOAD_SIZE) {
        log_w("Payload too long, expected less than %d bytes, found %d", PAYLOAD_SIZE, record.getPayloadLength());
        return false;
    }
    if (record.getTnf() != EXPECTED_TNF_VALUE) {
        log_w("Wrong TNF value, expected %d, found %d", EXPECTED_TNF_VALUE, record.getTnf());
        return false;
    }
    return true;
}

uint8_t NfcHandler::setRecord(NdefRecord& record, byte* payload) {
    if (payload == nullptr) {
        log_w("Payload is null");
        return false;
    }
    record.setTnf(EXPECTED_TNF_VALUE);
    record.setPayload(payload, PAYLOAD_SIZE);
    
    return true;
}

/*
===================================================================================================================
                                                Public Methods
====================================================================================================================
*/


/**
 * This object represents the NFC antenna handler and is responsible for managing
 * communication with the NFC hardware using the provided I2C interface.
 * 
 */
NfcHandler nfcAntenna(&Wire);

uint8_t NfcHandler::begin() {
    if (!nfcAdapter.begin()) {
        return ERROR_PN532_INIT_FAILED;
    }
    return CODE_SUCCESS;
}

void NfcHandler::getLastPayload(byte* payload) {
    memcpy(payload, lastPayload, PAYLOAD_SIZE);
}

String NfcHandler::getLastPayloadString() {
    String lastPayloadString = "";
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        lastPayloadString += String(lastPayload[i], HEX) + " ";
    }
    return lastPayloadString;
}


uint8_t NfcHandler::readNfcTag(byte* payload) {
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
        memcpy(payload, lastPayload, PAYLOAD_SIZE);
    }

    return true;
}

uint8_t NfcHandler::writeNfcTag(byte* payload) {
    NfcTag tag;
    NdefMessage message = NdefMessage();
    NdefRecord record = NdefRecord();

    if (!setRecord(record, payload)) {
        return false;
    }

    if (!formatNfcMessage(message, record)) {
        return false;
    }

    if(!getNfcTag(tag)) {
        return false;
    }

    if (!nfcAdapter.write(message)) {
        log_e("Failed to write NDEF message to tag");
        return false;
    }

    log_d("NDEF message written to tag successfully");
    return true;
}