#ifndef MEMORYCHIP_HPP
#define MEMORYCHIP_HPP

#include <stdint.h>
#include "channelio.hpp"
#include "fastpins.hpp"

const uint16_t UNKNOWN_SIZE = 0;
struct MemoryChipProperties
{
    bool canReadAndWrite;
    uint16_t size;
    bool isNonVolatile;
    bool isSlow;
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

    bool getPropertiesAreKnown();
    MemoryChipProperties getProperties();
    void setProperties(MemoryChipProperties properties);
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

    bool _propertiesAreKnown = false;
    MemoryChipProperties _properties = {false, UNKNOWN_SIZE, false, false};

    bool _testAddress(uint16_t address, bool slow);
    bool _testNonVolatility();
};

#endif
