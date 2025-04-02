#include "NfcHandler.h"

/*
===================================================================================================================
                                                Private Methods
====================================================================================================================
*/

NfcHandler::NfcHandler(TwoWire* wire) 
    : pn532_i2c(*wire), nfcAdapter(pn532_i2c) {}


/**
 * @brief Reads an NFC tag if present and stores it in the provided tag object.
 * 
 * @param[out] tag A reference to an NfcTag object where the read tag data will be stored.
 * @return uint8_t Returns CODE_SUCCESS if the operation is successful, or
 *                 an error code indicating the failure reason.
 */
uint8_t NfcHandler::getNfcTag(NfcTag& tag) {

    if (!nfcAdapter.tagPresent()) {
        return ERROR_NFC_TAG_NOT_PRESENT;
    }

    tag = nfcAdapter.read();

    return CODE_SUCCESS;
}

/**
 * @brief Retrieves the NDEF message from the provided NFC tag.
 * 
 * @param tag The NFC tag from which the NDEF message will be retrieved.
 * @param message A reference to an NdefMessage object where the retrieved
 *                NDEF message will be stored.
 * @return uint8_t Returns CODE_SUCCESS if the operation is successful, or
 *                 an error code indicating the failure reason.
 */
uint8_t NfcHandler::getNdefMessage(NfcTag& tag, NdefMessage& message) {

    errorCode = isNfcTagValid(tag);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    message = tag.getNdefMessage();

    return CODE_SUCCESS;
}

/**
 * @brief Extracts the payload from the first record of an NDEF message and stores it in a byte array.
 * 
 * @param message The NDEF message containing one or more records.
 * @param payload A pointer to a byte array where the payload will be stored. Can be nullptr.
 * @return uint8_t Returns CODE_SUCCESS if the operation is successful, or
 *                 an error code indicating the failure reason.
 * 
 */
uint8_t NfcHandler::getPayload(NdefMessage& message, byte* payload) {
    NdefRecord record;

    errorCode = isNdefMessageValid(message);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    // Get the first record from the NDEF message
    // If there are multiple records, you can loop through them
    record = message.getRecord(0);

    errorCode = isRecordValid(record);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }
    
    record.getPayload(payload);

    return CODE_SUCCESS;
}

/**
 * @brief Validates the given NFC tag by checking its UID and NDEF message.
 * 
 * @param tag Reference to the NfcTag object to be validated.
 * @return uint8_t Returns CODE_SUCCESS if the operation is successful, or
 *                 an error code indicating the failure reason.
 * 
 */
uint8_t NfcHandler::isNfcTagValid(NfcTag& tag) {

    // Check if the tag given is valid by checking the UID
    if (tag.getUidString() == "") {
        return ERROR_INVALID_UID;
    }

    if (!tag.hasNdefMessage()) {
        return ERROR_NO_NDEF_MESSAGE;
    }
    
    return CODE_SUCCESS;
}

/**
 * @brief Validates the number of records in an NDEF message.
 *
 *
 * @param message The NDEF message to validate.
 * @return uint8_t Returns CODE_SUCCESS if the operation is successful, or
 *                 an error code indicating the failure reason.
 * @note Refer to the header file for more details on the CardID format and
 *       expected TNF value.
 */
uint8_t NfcHandler::isNdefMessageValid(NdefMessage message) {
    unsigned int recordCount = message.getRecordCount();

    if (recordCount < NUM_RECORDS) {
        return ERROR_NOT_ENOUGH_RECORDS;
    }
    
    if (recordCount > NUM_RECORDS) {
        return ERROR_TOO_MANY_RECORDS;
    }
    return CODE_SUCCESS;
}

/**
 * @brief Validates an NDEF record by checking its payload length and TNF value.
 * 
 * @param record The NDEF record to validate.
 * @return uint8_t Returns CODE_SUCCESS if the operation is successful, or
 *                 an error code indicating the failure reason.
 * 
 * @note Refer to the header file for more details on the CardID format and
 *       expected TNF value.
 */
uint8_t NfcHandler::isRecordValid(NdefRecord record) {

    if (record.getPayloadLength() > PAYLOAD_SIZE) {
        return ERROR_INVALID_PAYLOAD;
    }

    if (record.getTnf() != EXPECTED_TNF_VALUE) {
        return ERROR_INVALID_TNF;
    }

    return CODE_SUCCESS;
}

/**
 * @brief Sets the properties of an NDEF record with the given payload.
 * 
 * @param record Reference to the NdefRecord object to be configured.
 * @param payload Pointer to the byte array containing the payload data.
 *                Must not be nullptr.
 * 
 * @return uint8_t Returns CODE_SUCCESS if the record is successfully set.
 *                 Returns ERROR_INVALID_PAYLOAD if the payload is nullptr.
 * 
 */
uint8_t NfcHandler::setRecord(NdefRecord& record, byte* payload) {

    if (payload == nullptr) {
        return ERROR_INVALID_PAYLOAD;
    }

    record.setTnf(EXPECTED_TNF_VALUE);
    record.setPayload(payload, PAYLOAD_SIZE);
    
    return CODE_SUCCESS;
}

/**
 * @brief Add record to the NDEF message after verifying its validity.
 * 
 * @param message Reference to the NdefMessage object where the record will be added.
 * @param record Reference to the NdefRecord object to be validated and added to the message.
 * @return uint8_t Returns an error code if the record is invalid, otherwise returns CODE_SUCCESS.
 */
uint8_t NfcHandler::setNDEF(NdefMessage& message, NdefRecord& record) {

    errorCode = isRecordValid(record);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    message.addRecord(record);

    return CODE_SUCCESS;
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

/**
 * @brief Initializes the NFC adapter and checks if it is ready for use.
 * 
 * @return uint8_t Returns CODE_SUCCESS if the initialization is successful,
 *                 or an error code indicating the failure reason.
 */
uint8_t NfcHandler::begin(void) {
    bool success;

    success = nfcAdapter.begin();

    if (!success) {
        return ERROR_PN532_INIT_FAILED;
    }

    return CODE_SUCCESS;
}

/**
 * @brief Retrieves the last payload read from the NFC tag and copies it to the provided pointer.
 * 
 * @param payload Pointer to a byte array where the last payload will be copied.
 *                Must not be nullptr.
 * @return uint8_t Returns CODE_SUCCESS if the operation is successful,
 *                 or an error code indicating the failure reason.
 */
uint8_t NfcHandler::getLastPayload(byte* payload) {
    // Check if the payload pointer is not null before copying
    // to avoid dereferencing a null pointer.
    if (payload == nullptr) {
        return ERROR_INVALID_PAYLOAD;
    }

    memcpy(payload, lastPayload, PAYLOAD_SIZE);
    
    return CODE_SUCCESS;
}

/**
 * @brief Converts the last payload to a string representation.
 * 
 * @return String Returns a string representation of the last payload.
 * 
 * @note The payload is represented as a hexadecimal string.
 */
String NfcHandler::getLastPayloadString(void) {
    String lastPayloadString = "";

    // Convert the last payload to a string representation
    for (int i = 0; i < PAYLOAD_SIZE; i++) {
        lastPayloadString += String(lastPayload[i], HEX) + " ";
    }

    return lastPayloadString;
}

/**
 * @brief Reads the NFC tag and retrieves its payload.
 * 
 * @param payload Pointer to a byte array where the payload will be stored.
 *                If nullptr, the last payload will be used.
 * @return uint8_t Returns CODE_SUCCESS if the operation is successful,
 *                 or an error code indicating the failure reason.
 */
uint8_t NfcHandler::readNfcTag(byte* payload) {
    NfcTag tag;
    NdefMessage message;

    errorCode = getNfcTag(tag);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    errorCode = getNdefMessage(tag, message);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    errorCode = getPayload(message, lastPayload);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    // Copy the last payload to the provided pointer if it's not null
    if (payload) {
        memcpy(payload, lastPayload, PAYLOAD_SIZE);
    }

    return CODE_SUCCESS;
}

/**
 * @brief Writes the given payload to an NFC tag.
 * 
 * @param payload Pointer to a byte array containing the payload to be written.
 *                Must not be nullptr.
 * @return uint8_t Returns CODE_SUCCESS if the operation is successful,
 *                 or an error code indicating the failure reason.
 */
uint8_t NfcHandler::writeNfcTag(byte* payload) {
    NfcTag tag;
    NdefMessage message = NdefMessage();
    NdefRecord record = NdefRecord();

    errorCode = setRecord(record, payload);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    errorCode = setNDEF(message, record);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    errorCode = getNfcTag(tag);

    if (errorCode != CODE_SUCCESS) {
        return errorCode;
    }

    // Write the NDEF message to the NFC tag
    bool success = nfcAdapter.write(message);

    if (!success) {
        return ERROR_COULD_NOT_WRITE_TAG;
    }

    log_i("NDEF message written to NFC tag successfully.");
    return CODE_SUCCESS;
}