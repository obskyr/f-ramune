#ifndef SERIALINTERFACE_HPP
#define SERIALINTERFACE_HPP

#include <Arduino.h>
#include "memorychip.hpp"

#define FRAMUNE_PROTOCOL_VERSION 0

class SerialInterface
{
public:
    SerialInterface(Stream* serial, MemoryChip* memoryChip);
    bool update();
private:
    int _readByteWithTimeout(uint8_t& n);
    int _readUint32WithTimeout(uint32_t& n);
    void _writeUint16(uint16_t n);
    void _writeUint32(uint32_t n);
    bool _checkForCommand();
    int _receiveMemoryChipProperties(
        MemoryChipKnownProperties& knownProperties,
        MemoryChipProperties& properties
    );
    void _sendMemoryChipProperties(
        MemoryChipKnownProperties& knownProperties,
        MemoryChipProperties& properties
    );

    enum class SerialState
    {
        WAITING_FOR_COMMAND
    };

    enum class SerialCommand : uint8_t
    {
        GET_VERSION,
        SET_AND_ANALYZE_CHIP
    };

    Stream* _serial;
    MemoryChip* _memoryChip;
    SerialState _state;
};

#endif
