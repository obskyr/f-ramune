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

int SerialInterface::_readByteWithTimeout(uint8_t &b)
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
    b = _serial->read();
    return 0;
}

void SerialInterface::_write_uint16(uint16_t n)
{
    _serial->write(static_cast<uint8_t>(n >> 8));
    _serial->write(static_cast<uint8_t>(n));
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
            _write_uint16(FRAMUNE_PROTOCOL_VERSION);
            break;
        }
    }
    return false;
}
