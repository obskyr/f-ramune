#include "memorychip.hpp"

#include <stdint.h>
#include "fastpins.hpp"

// A little warning for you: to maximize speed, the read and write functions
// don't verify that the pins are in the correct mode - make sure to manage
// switchToReadMode and switchToWriteMode properly.

MemoryChip::MemoryChip(OutputChannel<uint16_t>* addressChannel,
                       InputOutputChannel<uint8_t>* dataChannel,
                       unsigned int cePin, unsigned int oePin,
                       unsigned int wePin, unsigned int powerPin,
                       uint8_t powerPinOnState) :
    _addressChannel(addressChannel), _dataChannel(dataChannel),
    _cePin(pinToPortInfo(cePin)), _oePin(pinToPortInfo(oePin)),
    _wePin(pinToPortInfo(wePin)), _powerPin(pinToPortInfo(powerPin)),
    _powerPinOnState(powerPinOnState) {}

void MemoryChip::initPins()
{
    _addressChannel->initOutput();
    switchToReadMode();

    // It's important that these pins immediately
    // start high, so the chip isn't enabled!
    SET_BITS_IN_PORT_HIGH(_cePin.out, _cePin.bitMask);
    pinMode(_cePin.pin, OUTPUT);
    SET_BITS_IN_PORT_HIGH(_oePin.out, _oePin.bitMask);
    pinMode(_oePin.pin, OUTPUT);
    SET_BITS_IN_PORT_HIGH(_wePin.out, _wePin.bitMask);
    pinMode(_wePin.pin, OUTPUT);

    pinMode(_powerPin.pin, OUTPUT);
    powerOn();
}

void MemoryChip::powerOff()
{
    if (_powerPinOnState == HIGH) {
        // If the power is on high, that means a low-side switching circuit
        // is used, which means ground is being cut off. When ground is cut
        // off, to cut power to the chip, all other lines need to be high.
        _addressChannel->output(0xFFFF);
        if (_inWriteMode) {
            _dataChannel->output(0xFF);
        }
        SET_BITS_IN_PORT_LOW(_powerPin.out, _powerPin.bitMask);
    } else {
        // If the power is on high, that means a high-side switching circuit
        // is used, which means V+ is being cut off. When V+ is cut
        // off, to cut power to the chip, all other lines need to be low.
        _addressChannel->output(0);
        if (_inWriteMode) {
            _dataChannel->output(0);
        }
        SET_BITS_IN_PORT_LOW(_oePin.out, _oePin.bitMask);
        SET_BITS_IN_PORT_LOW(_wePin.out, _wePin.bitMask);
        SET_BITS_IN_PORT_HIGH(_powerPin.out, _powerPin.bitMask);
        // CE needs to go low last, since asserting CE low activates the chip.
        // By eliminating the last source of positive voltage with CE, we
        // minimize the risk of accidentally writing to the chip on power off.
        SET_BITS_IN_PORT_LOW(_cePin.out, _cePin.bitMask);
    }
    /*
        This delay is to accommodate MOSFET switching time.
        
        This generous value was chosen based on a supply voltage of 4.5 V,
        a max gate charge (Qg) of 75 nC, a 100 Ω series gate resistor
        (which limits current to 4.5 V / 100 Ω = 45 mA),
        and a max gate threshold (Vgs(th)) of 3 V.
        The minimum charging current then is (4.5 V - 3 V) / 100 Ω = 15 mA,
        which makes the max switch time 75 nC / 15 mA = 5 µs.
        These values are pretty unrealistic - the IRL520N in f-ramune's
        schematic has a Qg of 20 nC and a max Vgs(th) of 2 V, which given
        the other numbers really only requires 0.8 µs. Leeway! ☆

        Pretty sure powerOff and powerOn won't have to run in a tight
        loop, but if they ever do, this might need to be looked at.
    */
    delayMicroseconds(5);
}

void MemoryChip::powerOn()
{
    if (_powerPinOnState == HIGH) {
        SET_BITS_IN_PORT_HIGH(_powerPin.out, _powerPin.bitMask);
    } else {
        // For the same reason outlined in powerOff,
        // CE needs to go high before the V+ line does.
        SET_BITS_IN_PORT_HIGH(_cePin.out, _cePin.bitMask);
        SET_BITS_IN_PORT_LOW(_powerPin.out, _powerPin.bitMask);
        SET_BITS_IN_PORT_HIGH(_oePin.out, _oePin.bitMask);
        SET_BITS_IN_PORT_HIGH(_wePin.out, _wePin.bitMask);
    }
    delayMicroseconds(5);
}

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
    _properties = {false, 0, false, false};
    _propertiesAreKnown = true;

    if (_testAddress(0, false)) {
        _properties.canReadAndWrite = true;
    } else if (_testAddress(0, true)) {
        _properties.canReadAndWrite = true;
        _properties.isSlow = true;
    } else {
        return;
    }

    for (_properties.size = 0x8000; _properties.size > 0; _properties.size >>= 1) {
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
    (void) slow; // TODO: Implement EEPROM speed.

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
    
    bool isNonVolatile = true;
    for (uint16_t address = 0; address < testLength; address++) {
        if (readByte(address) != 0x22) {
            isNonVolatile = false;
            break;
        }
    }

    // It's like we were never there.
    writeBytes(0, prevBytes, testLength);
    delete[] prevBytes;

    return isNonVolatile;
}

void MemoryChip::switchToReadMode()
{
    _inWriteMode = false;
    _dataChannel->initInput();
}

uint8_t MemoryChip::readByte(uint16_t address)
{
    // TODO: Implement slow mode here and there and everywhere.
    _addressChannel->output(address);
    SET_BITS_IN_PORT_LOW(_cePin.out, _cePin.bitMask);
    SET_BITS_IN_PORT_LOW(_oePin.out, _oePin.bitMask);
    uint8_t data = _dataChannel->input();
    SET_BITS_IN_PORT_HIGH(_cePin.out, _cePin.bitMask);
    SET_BITS_IN_PORT_HIGH(_oePin.out, _oePin.bitMask);
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
    _inWriteMode = true;
    _dataChannel->initOutput();
}

void MemoryChip::writeByte(uint16_t address, uint8_t data)
{
    // MemoryChip::writeByte benchmarks with different _addressChannel types
    // (16 MHz ATmega328P, _dataChannel is always InputOutput_Port):
    // * digitalWrite: 250+ µs
    // * Output_ShiftRegister (bit-banging ports): ~112 µs
    // * Output_ShiftRegister (with hard-coded ports): ~77 µs
    // * Output_SpiShiftRegister: ~27 µs
    // Holy heck! SPI brought it down like crazy! That is way under the
    // target <69 µs that a serial baud rate of 115200 requires!

    _addressChannel->output(address);
    _dataChannel->output(data);
    // WE is active when CE is activated, so we're doing a CE-controlled write.
    SET_BITS_IN_PORT_LOW(_wePin.out, _wePin.bitMask);
    SET_BITS_IN_PORT_LOW(_cePin.out, _cePin.bitMask);
    SET_BITS_IN_PORT_HIGH(_cePin.out, _cePin.bitMask);
    SET_BITS_IN_PORT_HIGH(_wePin.out, _wePin.bitMask);
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
