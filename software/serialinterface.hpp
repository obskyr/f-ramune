#ifndef SERIALINTERFACE_HPP
#define SERIALINTERFACE_HPP

#include <Arduino.h>
#include <CRC32.h>
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
    bool _commandSetAndAnalyzeChip();
    int _receiveMemoryChipProperties(
        MemoryChipKnownProperties& knownProperties,
        MemoryChipProperties& properties
    );
    void _sendMemoryChipProperties(
        MemoryChipKnownProperties& knownProperties,
        MemoryChipProperties& properties
    );
    bool _commandRead();

    enum class SerialState
    {
        WAITING_FOR_COMMAND,
        READING
    };

    enum class SerialCommand : uint8_t
    {
        GET_VERSION,
        SET_AND_ANALYZE_CHIP,
        READ
    };

    Stream* _serial;
    MemoryChip* _memoryChip;
    SerialState _state;

    uint16_t _currentAddress;
    uint32_t _currentBytesLeft;
    CRC32 _currentCrc32;
};

#endif
