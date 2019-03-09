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
    case SerialState::READING:
        return _stateReading();
        break;
    case SerialState::WRITING:
        return _stateWriting();
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
            return _commandSetAndAnalyzeChip();
            break;
        case static_cast<uint8_t>(SerialCommand::READ):
            return _commandRead();
            break;
        case static_cast<uint8_t>(SerialCommand::WRITE):
            return _commandWrite();
            break;
        }
    }
    return false;
}

bool SerialInterface::_commandSetAndAnalyzeChip()
{
    MemoryChipKnownProperties receivedKnownProperties;
    MemoryChipProperties receivedProperties;
    if (_receiveMemoryChipProperties(receivedKnownProperties,
                                     receivedProperties) != 0) {
        return false;
    }
    _memoryChip->setProperties(&receivedKnownProperties,
                               &receivedProperties);
    _memoryChip->analyzeUnknownProperties();
    _memoryChip->getProperties(&receivedKnownProperties,
                               &receivedProperties);
    _sendMemoryChipProperties(receivedKnownProperties, receivedProperties);
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

int SerialInterface::_readAddressAndSize(uint16_t& address, uint32_t& size)
{
    uint32_t address32Bits;

    int errorCode;
    if ((errorCode = _readUint32WithTimeout(address32Bits)) != 0) {return errorCode;}
    if ((errorCode = _readUint32WithTimeout(size)) != 0) {return errorCode;}

    MemoryChipKnownProperties knownProperties;
    MemoryChipProperties properties;
    _memoryChip->getProperties(&knownProperties, &properties);

    if (address32Bits > 0xFFFF) {
        size = 0;
        address = 0;
    } else {
        address = static_cast<uint16_t>(address32Bits);
    }
    if (knownProperties.size) {
        if (address > properties.size - 1) {
            size = 0;
        } else if (properties.size - address < size) {
            size = properties.size - address;
        }
    }

    return 0;
}

bool SerialInterface::_commandRead()
{
    uint16_t address;
    uint32_t size;
    if (_readAddressAndSize(address, size) != 0) {return false;}
    _writeUint32(size);
    
    _currentOperationStart = address;
    _currentAddress = address;
    _currentOperationSize = size;
    _currentBytesLeft = size;
    _currentCrc32.reset();
    _memoryChip->switchToReadMode();
    _state = SerialState::READING;

    return true;
}

bool SerialInterface::_stateReading()
{
    if (_currentBytesLeft) {
        uint8_t n = _memoryChip->readByte(_currentAddress);
        _currentCrc32.update(n);
        _serial->write(n);
        _currentAddress++;
        _currentBytesLeft--;
        return true;
    } else {
        _writeUint32(_currentCrc32.finalize());
        _state = SerialState::WAITING_FOR_COMMAND;
        return false;
    }
}

bool SerialInterface::_commandWrite()
{
    MemoryChipKnownProperties knownProperties;
    MemoryChipProperties properties;
    _memoryChip->getProperties(&knownProperties, &properties);
    // Unused at the moment.
    _serial->write(knownProperties.isSlow && properties.isSlow);

    uint16_t address;
    uint32_t size;
    if (_readAddressAndSize(address, size) != 0) {return false;}
    _writeUint32(size);

    _currentOperationStart = address;
    _currentAddress = address;
    _currentOperationSize = size;
    _currentBytesLeft = size;
    _currentCrc32.reset();
    _memoryChip->switchToWriteMode();
    _state = SerialState::WRITING;

    return true;
}

bool SerialInterface::_stateWriting()
{
    if (_currentBytesLeft) {
        uint8_t n;
        if (_readByteWithTimeout(n) != 0) {
            _state = SerialState::WAITING_FOR_COMMAND;
            return false;
        }
        _memoryChip->writeByte(_currentAddress, n);
        _currentAddress++;
        _currentBytesLeft--;
        return true;
    } else {
        _memoryChip->switchToReadMode();
        uint32_t end = _currentOperationStart + _currentOperationSize;
        bool all_bytes_seem_pulled = true;
        for (uint32_t address = _currentOperationStart; address < end; address++) {
            uint8_t n = _memoryChip->readByte(address);
            _currentCrc32.update(n);
            if (n != 0xFF && n != 0x00) {
                all_bytes_seem_pulled = false;
            }
        }
        _writeUint32(_currentCrc32.finalize());

        // If all the bytes written were 0x00 or 0xFF, and the data lines have
        // pull-downs or pull-ups (respectively) on them, it's impossible to
        // tell whether the data was written successfully without performing
        // an extra write like this.
        uint8_t errorCode = 0;
        if (all_bytes_seem_pulled) {
            _memoryChip->switchToReadMode();
            uint8_t prevByte = _memoryChip->readByte(_currentOperationStart);
            _memoryChip->switchToWriteMode();
            _memoryChip->writeByte(_currentOperationStart, 0xA5);
            _memoryChip->switchToReadMode();
            if (_memoryChip->readByte(_currentOperationStart) != 0xA5) {
                errorCode = 1;
            }
            _memoryChip->switchToWriteMode();
            _memoryChip->writeByte(_currentOperationStart, prevByte);
        }
        _serial->write(errorCode);

        _state = SerialState::WAITING_FOR_COMMAND;
        return false;
    }
}
