#include "NfcHandler.h"

NfcHandler nfcAntenna(&Wire);

NfcHandler::NfcHandler(TwoWire* wire) {
    pn532_i2c = PN532_I2C(Wire);
    nfcAdapter = NfcAdapter(pn532_i2c);
}

bool NfcHandler::begin() {
    nfcAdapter.begin();
}
