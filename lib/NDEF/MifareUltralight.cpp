#include <MifareUltralight.h>

#define ULTRALIGHT_PAGE_SIZE 4
#define ULTRALIGHT_READ_SIZE 4 // we should be able to read 16 bytes at a time

#define ULTRALIGHT_DATA_START_PAGE 4
#define ULTRALIGHT_MESSAGE_LENGTH_INDEX 1
#define ULTRALIGHT_DATA_START_INDEX 2
#define ULTRALIGHT_MAX_PAGE 63

#define NFC_FORUM_TAG_TYPE_2 ("NFC Forum Type 2")

MifareUltralight::MifareUltralight(PN532& nfcShield)
{
    nfc = &nfcShield;
    ndefStartIndex = 0;
    messageLength = 0;
}

MifareUltralight::~MifareUltralight()
{
}

NfcTag MifareUltralight::read(byte * uid, unsigned int uidLength)
{
    if (isUnformatted())
    {
        log_w("WARNING: Tag is not formatted.");
        return NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_2);
    }

    readCapabilityContainer(); // meta info for tag
    findNdefMessage();
    calculateBufferSize();

    if (messageLength == 0) { // data is 0x44 0x03 0x00 0xFE
        NdefMessage message = NdefMessage();
        message.addEmptyRecord();
        return NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_2, message);
    }

    boolean success;
    uint8_t page;
    uint8_t index = 0;
    byte buffer[bufferSize];
    for (page = ULTRALIGHT_DATA_START_PAGE; page < ULTRALIGHT_MAX_PAGE; page++)
    {
        // read the data
        success = nfc->mifareultralight_ReadPage(page, &buffer[index]);
        if (success)
        {
            log_d("Page %d", page);
            nfc->PrintHexChar(&buffer[index], ULTRALIGHT_PAGE_SIZE);
        }
        else
        {
            log_e("Read failed %d", page);
            // TODO error handling
            messageLength = 0;
            break;
        }

        if (index >= (messageLength + ndefStartIndex))
        {
            break;
        }

        index += ULTRALIGHT_PAGE_SIZE;
    }

    NdefMessage ndefMessage = NdefMessage(&buffer[ndefStartIndex], messageLength);
    return NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_2, ndefMessage);

}

boolean MifareUltralight::isUnformatted()
{
    uint8_t page = 4;
    byte data[ULTRALIGHT_READ_SIZE];
    boolean success = nfc->mifareultralight_ReadPage (page, data);
    if (success)
    {
        return (data[0] == 0xFF && data[1] == 0xFF && data[2] == 0xFF && data[3] == 0xFF);
    }
    else
    {
        log_e("Error. Failed read page %d", page);
        return false;
    }
}

// page 3 has tag capabilities
void MifareUltralight::readCapabilityContainer()
{
    byte data[ULTRALIGHT_PAGE_SIZE];
    int success = nfc->mifareultralight_ReadPage (3, data);
    if (success)
    {
        // See AN1303 - different rules for Mifare Family byte2 = (additional data + 48)/8
        tagCapacity = data[2] * 8;
        log_d("Tag capacity %d bytes", tagCapacity);

        // TODO future versions should get lock information
    }
}

// read enough of the message to find the ndef message length
void MifareUltralight::findNdefMessage()
{
    int page;
    byte data[12]; // 3 pages
    byte* data_ptr = &data[0];

    // the nxp read command reads 4 pages, unfortunately adafruit give me one page at a time
    boolean success = true;
    for (page = 4; page < 6; page++)
    {
        success = success && nfc->mifareultralight_ReadPage(page, data_ptr);
        log_d("Page %d - ", page);
        nfc->PrintHexChar(data_ptr, 4);
        data_ptr += ULTRALIGHT_PAGE_SIZE;
    }

    if (success)
    {
        if (data[0] == 0x03)
        {
            messageLength = data[1];
            ndefStartIndex = 2;
        }
        else if (data[5] == 0x3) // page 5 byte 1
        {
            // TODO should really read the lock control TLV to ensure byte[5] is correct
            messageLength = data[6];
            ndefStartIndex = 7;
        }
    }

    log_d("messageLength %d", messageLength);
    log_d("ndefStartIndex %d", ndefStartIndex);
}

// buffer is larger than the message, need to handle some data before and after
// message and need to ensure we read full pages
void MifareUltralight::calculateBufferSize()
{
    // TLV terminator 0xFE is 1 byte
    bufferSize = messageLength + ndefStartIndex + 1;

    if (bufferSize % ULTRALIGHT_READ_SIZE != 0)
    {
        // buffer must be an increment of page size
        bufferSize = ((bufferSize / ULTRALIGHT_READ_SIZE) + 1) * ULTRALIGHT_READ_SIZE;
    }
}

boolean MifareUltralight::write(NdefMessage& m, byte * uid, unsigned int uidLength)
{
    if (isUnformatted())
    {
        log_w("WARNING: Tag is not formatted.");
        return false;
    }
    readCapabilityContainer(); // meta info for tag

    messageLength  = m.getEncodedSize();
    ndefStartIndex = messageLength < 0xFF ? 2 : 4;
    calculateBufferSize();

    if(bufferSize>tagCapacity) {
        log_e("Encoded Message length exceeded tag Capacity %d", tagCapacity);
        return false;
    }

    uint8_t encoded[bufferSize];
    uint8_t *  src = encoded;
    unsigned int position = 0;
    uint8_t page = ULTRALIGHT_DATA_START_PAGE;

    // Set message size. With ultralight should always be less than 0xFF but who knows?

    encoded[0] = 0x3;
    if (messageLength < 0xFF)
    {
        encoded[1] = messageLength;
    }
    else
    {
        encoded[1] = 0xFF;
        encoded[2] = ((messageLength >> 8) & 0xFF);
        encoded[3] = (messageLength & 0xFF);
    }

    m.encode(encoded+ndefStartIndex);
    // this is always at least 1 byte copy because of terminator.
    memset(encoded+ndefStartIndex+messageLength,0,bufferSize-ndefStartIndex-messageLength);
    encoded[ndefStartIndex+messageLength] = 0xFE; // terminator

    log_d("messageLength %d", messageLength);
    log_d("Tag Capacity %d", tagCapacity);
    nfc->PrintHex(encoded,bufferSize);

    while (position < bufferSize){ //bufferSize is always times pagesize so no "last chunk" check
        // write page
        if (!nfc->mifareultralight_WritePage(page, src))
            return false;
        log_d("Wrote page %d - ", page);
        nfc->PrintHex(src,ULTRALIGHT_PAGE_SIZE);
        page++;
        src+=ULTRALIGHT_PAGE_SIZE;
        position+=ULTRALIGHT_PAGE_SIZE;
    }
    return true;
}

// Mifare Ultralight can't be reset to factory state
// zero out tag data like the NXP Tag Write Android application
boolean MifareUltralight::clean()
{
    readCapabilityContainer(); // meta info for tag

    uint8_t pages = (tagCapacity / ULTRALIGHT_PAGE_SIZE) + ULTRALIGHT_DATA_START_PAGE;

    // factory tags have 0xFF, but OTP-CC blocks have already been set so we use 0x00
    uint8_t data[4] = { 0x00, 0x00, 0x00, 0x00 };

    for (int i = ULTRALIGHT_DATA_START_PAGE; i < pages; i++) {
        log_d("Wrote page %d - ", i);
        nfc->PrintHex(data, ULTRALIGHT_PAGE_SIZE);
        if (!nfc->mifareultralight_WritePage(i, data)) {
            return false;
        }
    }
    return true;
}
