#ifndef SERIALINTERFACE_HPP
#define SERIALINTERFACE_HPP

#include <Arduino.h>
#include "memorychip.hpp"

#define FRAMUNE_PROTOCOL_VERSION 0

enum class SerialState
{
    WAITING_FOR_COMMAND
};

enum class SerialCommand : uint8_t
{
    GET_VERSION
};

class SerialInterface
{
public:
    SerialInterface(Stream* serial, MemoryChip* memoryChip);
    bool update();
private:
    int _readByteWithTimeout(uint8_t &b);
    void _write_uint16(uint16_t n);
    bool _checkForCommand();

    Stream* _serial;
    MemoryChip* _memoryChip;
    SerialState _state;
};

#endif
