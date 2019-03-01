#ifndef MEMORYCHIP_HPP
#define MEMORYCHIP_HPP

#include <stdint.h>
#include "channelio.hpp"
#include "fastpins.hpp"

// MemoryChip::analyze() will check sizes between these two bit widths.
#define MEMORY_CHIP_MAX_ADDRESS_WIDTH 17
#define MEMORY_CHIP_MIN_ADDRESS_WIDTH 8

struct MemoryChipProperties
{
    bool isOperational;
    uint32_t size;
    bool isNonVolatile;
    bool isSlow;
};

struct MemoryChipKnownProperties
{
    bool isOperational : 1;
    bool size : 1;
    bool isNonVolatile : 1;
    bool isSlow : 1;
};

class MemoryChip
{
public:
    MemoryChip(OutputChannel<uint16_t>* addressChannel,
               InputOutputChannel<uint8_t>* dataChannel,
               unsigned int cePin, unsigned int oePin,
               unsigned int wePin, unsigned int powerPin,
               uint8_t powerPinOnState);
    void initPins();

    void powerOff();
    void powerOn();

    void getProperties(MemoryChipKnownProperties* knownProperties,
                       MemoryChipProperties* properties);
    void setProperties(const MemoryChipKnownProperties* knownProperties,
                       const MemoryChipProperties* properties);
    void analyzeUnknownProperties();
    void analyze();
    
    void switchToReadMode();
    uint8_t readByte(uint16_t address);
    size_t readBytes(uint16_t address, uint8_t* dest, size_t length);

    void switchToWriteMode();
    void writeByte(uint16_t address, uint8_t data);
    size_t writeBytes(uint16_t address, uint8_t* source, size_t length);
private:
    OutputChannel<uint16_t>* _addressChannel;
    InputOutputChannel<uint8_t>* _dataChannel;
    PinPortInfo _cePin;
    PinPortInfo _oePin;
    PinPortInfo _wePin;
    PinPortInfo _powerPin;
    uint8_t _powerPinOnState;

    bool _inWriteMode = false;

    MemoryChipKnownProperties _knownProperties = {false, false, false, false};
    MemoryChipProperties _properties = {false, 0, false, false};

    bool _testAddress(uint16_t address, bool slow);
    bool _testNonVolatility();
};

#endif
