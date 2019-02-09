#include "memorychip.hpp"

#include <Arduino.h>

// A little warning for you: the read and write functions don't verify
// that the pins are in the correct mode - make sure to manage
// switchToReadMode and switchToWriteMode properly.

/*
    TODO: Use micros() to measure how fast this runs. If it can read/write a
    byte in under 69 microseconds, that means it can handle the target baud
    rate of 115200. If not, do one or more of the following:
    * Switch from digitalWrite to using ports (via a custom OutputChannel
      subclass, for example)
    * Lower the baud rate (but... slow...)
    * Chunk the serial transfer into 64 bytes at a time (although requesting
      the next chunk could potentially take mad time; investigate if pertinent)
*/

MemoryChip::MemoryChip(OutputChannel<uint16_t>* addressChannel,
                       InputOutputChannel<uint8_t>* dataChannel,
                       unsigned int cePin, unsigned int oePin,
                       unsigned int wePin, unsigned int powerPin) :
    _addressChannel(addressChannel), _dataChannel(dataChannel),
    _cePin(cePin), _oePin(oePin), _wePin(wePin), _powerPin(powerPin) {}

bool MemoryChip::getPropertiesAreKnown()
{
    return _propertiesAreKnown;
}

MemoryChipProperties MemoryChip::getProperties()
{
    return _properties;
}

void MemoryChip::setProperties(MemoryChipProperties properties)
{
    _properties = properties;
    _propertiesAreKnown = true;
}

void MemoryChip::analyze()
{
    _properties = {false, 0, false};
    _propertiesAreKnown = true;

    if (_testAddress(0, false)) {
        _properties.canReadAndWrite = true;
    } else if (_testAddress(0, true)) {
        _properties.canReadAndWrite = true;
        _properties.isSlow = true;
    } else {
        return;
    }

    for (_properties.size = 0x8000; _properties.size >>= 1; _properties.size > 0) {
        if (_testAddress(_properties.size - 1, _properties.isSlow)) {
            break;
        }
    }
    // If this is triggered, something went very very wrong...
    if (_properties.size == 0) {return;}

    _properties.isNonVolatile = _testNonVolatility();
}

bool MemoryChip::_testAddress(uint16_t address, bool slow)
{
    uint8_t prevByte = readByte(0);
    uint8_t testByte = prevByte == 0xA5 ? 0x5A : 0xA5;
    writeByte(address, testByte);
    uint8_t readBack = readByte(address);
    writeByte(address, prevByte);
    return readBack == testByte;
}

bool MemoryChip::_testNonVolatility()
{
    // Gotta fit in the MCU's RAM! 512 bytes is 1/4 of the Atmega328P's RAM,
    // so... if this ends up being too much, dial it down a bit.
    uint16_t testLength = 512;
    testLength = _properties.size < testLength ? _properties.size : testLength;
    uint8_t* prevBytes = new uint8_t[testLength];

    // Could be sped up by using a WE-controlled write to switch the bytes
    // as opposed to doing separate reads and writes, but... that'd
    // be some real #PrematureOptimization.
    readBytes(0, prevBytes, testLength);
    for (uint16_t address = 0; address < testLength; address++) {
        writeByte(address, 0x22); // Extremely arbitrarily chosen value!
    }

    powerOff();
    // TODO: Test how long SRAM actually takes to without a shadow
    // of a doubt have lost at least a bit of the test data.
    delay(2500);
    powerOn();
    
    _properties.isNonVolatile = true;
    for (uint16_t address = 0; address < testLength; address++) {
        if (readByte(address) != 0x22) {
            _properties.isNonVolatile = false;
            break;
        }
    }

    // It's like we were never there.
    writeBytes(0, prevBytes, testLength);
    delete[] prevBytes;
}

void MemoryChip::switchToReadMode()
{
    _dataChannel->initInput();
}

uint8_t MemoryChip::readByte(uint16_t address)
{
    // TODO: Implement slow mode here and there and everywhere.
    _addressChannel->output(address);
    digitalWrite(_cePin, LOW);
    digitalWrite(_oePin, LOW);
    uint8_t data = _dataChannel->input();
    digitalWrite(_cePin, HIGH);
    digitalWrite(_oePin, HIGH);
    return data;
}

size_t MemoryChip::readBytes(uint16_t address, uint8_t* dest, size_t length)
{
    size_t i;
    for (i = 0; i < length; i++) {
        dest[i] = readByte(address);
        address++;
    }
    // It's fairly worthless to return the number of written bytes here,
    // but it could be nice for consistency if there's ever a version of
    // this function that validates the length.
    // There might never be such a version, but hey. Hey.
    return i;
}

void MemoryChip::switchToWriteMode()
{
    _dataChannel->initOutput();
}

void MemoryChip::writeByte(uint16_t address, uint8_t data)
{
    _addressChannel->output(address);
    _dataChannel->output(data);
    // WE is active when CE is activated, so we're doing a CE-controlled write.
    digitalWrite(_wePin, LOW);
    digitalWrite(_cePin, LOW);
    digitalWrite(_cePin, HIGH);
    digitalWrite(_wePin, HIGH);
}

size_t MemoryChip::writeBytes(uint16_t address, uint8_t* source, size_t length)
{
    size_t i;
    for (i = 0; i < length; i++) {
        writeByte(address, source[i]);
        address++;
    }
    return i;
}
