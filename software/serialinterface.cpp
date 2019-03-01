#include "serialinterface.hpp"

SerialInterface::SerialInterface(Stream* serial, MemoryChip* memoryChip) :
    _serial(serial), _memoryChip(memoryChip) {}

bool SerialInterface::update()
{
    // Return true if busy (i.e. next update will continue a task), false if not.
    switch (_state) {
    case SerialState::WAITING_FOR_COMMAND:
        return _checkForCommand();
        break;
    }
    return false;
}

int SerialInterface::_readByteWithTimeout(uint8_t& n)
{
    if (!_serial->available()) {
        unsigned long int timeout = _serial->getTimeout();
        unsigned long int startedWaitingForByte = millis();
        while (!_serial->available()) {
            if (millis() - startedWaitingForByte >= timeout) {
                return 1;
            }
        }
    }
    n = _serial->read();
    return 0;
}

int SerialInterface::_readUint32WithTimeout(uint32_t& n)
{
    int errorCode;
    uint32_t x = 0;
    uint8_t b;
    for (int shift = (sizeof(uint32_t) - 1) * 8; shift >= 0; shift -= 8) {
        if ((errorCode = _readByteWithTimeout(b)) != 0) {
            return errorCode;
        }
        x |= static_cast<uint32_t>(b) << shift;
    }
    n = x;
    return 0;
}

// I couuuuld turn these two into a template, but it's not quite worth
// the structuring headache that C++ and the Arduino IDE impose together.
void SerialInterface::_writeUint16(uint16_t n)
{
    _serial->write(static_cast<uint8_t>(n >> 8));
    _serial->write(static_cast<uint8_t>(n));
}

void SerialInterface::_writeUint32(uint32_t n)
{
    for (int shift = (sizeof(uint32_t) - 1) * 8; shift >= 0; shift -= 8) {
        _serial->write(static_cast<uint8_t>(n >> shift));
    }
}

bool SerialInterface::_checkForCommand()
{
    if (_serial->available()) {
        uint8_t command = _serial->read();
        _serial->write(command);
        uint8_t ack;
        if (_readByteWithTimeout(ack) != 0) {return false;}
        if (ack != 0) {
            return false;
        }

        switch (command) {
        case static_cast<uint8_t>(SerialCommand::GET_VERSION):
            _writeUint16(FRAMUNE_PROTOCOL_VERSION);
            break;
        case static_cast<uint8_t>(SerialCommand::SET_AND_ANALYZE_CHIP):
            MemoryChipKnownProperties receivedKnownProperties;
            MemoryChipProperties receivedProperties;
            if (_receiveMemoryChipProperties(receivedKnownProperties,
                                             receivedProperties) != 0) {
                return false;
            }
            _memoryChip->setProperties(&receivedKnownProperties, &receivedProperties);
            // _memoryChip->analyzeUnknownProperties();
            _sendMemoryChipProperties(receivedKnownProperties, receivedProperties);
            break;
        }
    }
    return false;
}

int SerialInterface::_receiveMemoryChipProperties(
    MemoryChipKnownProperties& knownProperties,
    MemoryChipProperties& properties
)
{
    int errorCode;

    uint8_t n;
    if ((errorCode = _readByteWithTimeout(n)) != 0) {return errorCode;}
    knownProperties.isOperational = n;
    if ((errorCode = _readByteWithTimeout(n)) != 0) {return errorCode;}
    knownProperties.size = n;
    if ((errorCode = _readByteWithTimeout(n)) != 0) {return errorCode;}
    knownProperties.isNonVolatile = n;
    if ((errorCode = _readByteWithTimeout(n)) != 0) {return errorCode;}
    knownProperties.isSlow = n;

    if ((errorCode = _readByteWithTimeout(n)) != 0) {return errorCode;}
    properties.isOperational = n;
    if ((errorCode = _readUint32WithTimeout(properties.size)) != 0) {
        return errorCode;
    }
    if ((errorCode = _readByteWithTimeout(n)) != 0) {return errorCode;}
    properties.isNonVolatile = n;
    if ((errorCode = _readByteWithTimeout(n)) != 0) {return errorCode;}
    properties.isSlow = n;

    return 0;
}

void SerialInterface::_sendMemoryChipProperties(
    MemoryChipKnownProperties &knownProperties,
    MemoryChipProperties &properties
)
{
    _serial->write(knownProperties.isOperational);
    _serial->write(knownProperties.size);
    _serial->write(knownProperties.isNonVolatile);
    _serial->write(knownProperties.isSlow);

    _serial->write(properties.isOperational);
    _writeUint32(properties.size);
    _serial->write(properties.isNonVolatile);
    _serial->write(properties.isSlow);
}
