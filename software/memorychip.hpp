#ifndef MEMORYCHIP_HPP
#define MEMORYCHIP_HPP

#include <stdint.h>
#include "channelio.hpp"
#include "fastpins.hpp"

// MemoryChip::analyze() will check sizes between these two bit widths.
// To support memory chips of, say, up to 128 KiB, this max would need to be
// raised to 17, and MemoryChip would need to be a template that takes the
// integer type (uint32_t, in the case of 128 KiB) of its address as a
// parameter. In that case, it wouldn't make too much sense to have the same
// min/max for each type of MemoryChip, either.
#define MEMORY_CHIP_MAX_ADDRESS_WIDTH 16
#define MEMORY_CHIP_MIN_ADDRESS_WIDTH 8

// How many addresses a byte may be mirrored at, given the above parameters.
#define MEMORY_CHIP_POSSIBLE_MIRRORS ( \
    MEMORY_CHIP_MAX_ADDRESS_WIDTH - MEMORY_CHIP_MIN_ADDRESS_WIDTH + 1 \
)

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
    uint32_t _testSize();
    bool _testNonVolatility();
};

#endif
