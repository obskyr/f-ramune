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

    powerOff();
    pinMode(_powerPin.pin, OUTPUT);
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

void MemoryChip::getProperties(MemoryChipKnownProperties* knownProperties,
                               MemoryChipProperties* properties)
{
    *knownProperties = _knownProperties;
    *properties = _properties;
}

void MemoryChip::setProperties(const MemoryChipKnownProperties* knownProperties,
                               const MemoryChipProperties* properties)
{
    _knownProperties = *knownProperties;
    _properties = *properties;
}

void MemoryChip::analyzeUnknownProperties()
{
    // A decidedly non-operational chip doesn't have any properties, yo!
    if (_knownProperties.isOperational && !_properties.isOperational) {
        _knownProperties = {true, false, false, false};
        _properties = {false, 0, false, false};
        return;
    }

    bool wasInWriteMode = _inWriteMode;

    // This'll overwrite a "known" isOperational if it's set to true, but that
    // only makes sense - in testing isSlow, isOperational has to be tested,
    // and ignoring that result would be mad silly (if neither testing fast
    // nor slow works, we can't give isSlow a meaningful value, and we know
    // the chip isn't operational). It does, however, preserve a "known"
    // isSlow even if it tests OK for fast operation.
    if (!_knownProperties.isOperational || !_knownProperties.isSlow) {
        if (_testAddress(0, _knownProperties.isSlow && _properties.isSlow)) {
            _knownProperties.isOperational = true;
            _properties.isOperational = true;
            _knownProperties.isSlow = true;
            _properties.isSlow = _knownProperties.isSlow && _properties.isSlow;
        } else if (!_knownProperties.isSlow && _testAddress(0, true)) {
            _knownProperties.isOperational = true;
            _properties.isOperational = true;
            _knownProperties.isSlow = true;
            _properties.isSlow = true;
        } else {
            // Neither a fast test nor a slow test worked.
            _knownProperties = {true, false, false, false};
            _properties = {false, 0, false, false};
            if (wasInWriteMode) {
                switchToWriteMode();
            } else {
                switchToReadMode();
            }
            return;
        }
    }

    if (!_knownProperties.size) {
        uint32_t size = _testSize();
        if (size != 0) {
            // If this isn't triggered, something went very very wrong...
            _knownProperties.size = true;
        }
        _properties.size = size;
    }

    if (!_knownProperties.isNonVolatile) {
        _knownProperties.isNonVolatile = true;
        _properties.isNonVolatile = _testNonVolatility();
    }

    if (wasInWriteMode) {
        switchToWriteMode();
    } else {
        switchToReadMode();
    }
}

void MemoryChip::analyze()
{
    _knownProperties = {false, false, false, false};
    _properties = {false, 0, false, false};
    analyzeUnknownProperties();
}

bool MemoryChip::_testAddress(uint16_t address, bool slow)
{
    (void) slow; // TODO: Implement EEPROM speed.

    switchToReadMode();
    uint8_t prevByte = readByte(0);
    uint8_t testByte = prevByte == 0xA5 ? 0x5A : 0xA5;
    switchToWriteMode();
    writeByte(address, testByte);
    switchToReadMode();
    uint8_t readBack = readByte(address);
    switchToWriteMode();
    writeByte(address, prevByte);
    return readBack == testByte;
}

template <class T>
bool inArray(T arr[], size_t length, T element)
{
    for (size_t i = 0; i < length; i++) {
        if (arr[i] == element) {
            return true;
        }
    }
    return false;
}

uint32_t MemoryChip::_testSize()
{
    // We need to make sure that the byte we try writing isn't already at any
    // of the lower addresses we're going to check - otherwise the size could
    // be determined incorrectly depending on the data on the chip.
    switchToReadMode();
    uint8_t forbiddenTestBytes[MEMORY_CHIP_POSSIBLE_MIRRORS];
    const uint32_t maxAddress = (
        static_cast<uint32_t>(1) << MEMORY_CHIP_MAX_ADDRESS_WIDTH
    ) - 1;
    uint32_t testAddress = maxAddress;
    uint8_t prevByte = readByte(testAddress);
    for (int i = 0; i < MEMORY_CHIP_POSSIBLE_MIRRORS; i++) {
        // The very highest test address won't be checked.
        testAddress >>= 1;
        forbiddenTestBytes[i] = readByte(testAddress);
    }

    uint8_t testByte = 0x5A;
    while (inArray<uint8_t>(
        forbiddenTestBytes, MEMORY_CHIP_POSSIBLE_MIRRORS, testByte
    )) {
        testByte++;
    }

    testAddress = maxAddress;
    switchToWriteMode();
    writeByte(testAddress, testByte);
    // Checks the address width under the minimum address width too, to make
    // sure there aren't mirrored bytes below that too.
    switchToReadMode();
    for (int i = 0; i < MEMORY_CHIP_POSSIBLE_MIRRORS; i++) {
        testAddress >>= 1;
        if (readByte(testAddress) != testByte) {
            // Once again, we're in, we're out again, and no one gets hurt.
            switchToWriteMode();
            writeByte(maxAddress, prevByte);
            // 1 for the bit lost from the left shift; 1 for address → size.
            return (testAddress << 1) + 1 + 1;
        }
    }

    // Yeesh. You don't wanna end up here.
    // This means even the minimum address width is a mirrored byte.
    return 0;
}

bool MemoryChip::_testNonVolatility()
{
    // Gotta fit in the MCU's RAM! 512 bytes is 1/4 of the Atmega328P's RAM,
    // so... if this ends up being too much, dial it down a bit.
    uint16_t testLength;
    if (_knownProperties.size && _properties.size < 512) {
        testLength = _knownProperties.size;
    } else {
        testLength = 512;
    }
    testLength = _properties.size < testLength ? _properties.size : testLength;
    uint8_t* prevBytes = new uint8_t[testLength];

    // Could be sped up by using a WE-controlled write to switch the bytes
    // as opposed to doing separate reads and writes, but... that'd
    // be some real #PrematureOptimization.
    switchToReadMode();
    readBytes(0, prevBytes, testLength);
    switchToWriteMode();
    for (uint16_t address = 0; address < testLength; address++) {
        writeByte(address, 0x22); // Extremely arbitrarily chosen value!
    }

    powerOff();
    // On the SRAM chip I tested this with, 10 milliseconds was enough for most
    // of the data to have been reliably lost (~416 / 512 bytes). This might be
    // different for other SRAM chips, though, so it may need to be dialed up...
    delay(10);
    powerOn();
    
    switchToReadMode();
    bool isNonVolatile = true;
    for (uint16_t address = 0; address < testLength; address++) {
        if (readByte(address) != 0x22) {
            isNonVolatile = false;
            break;
        }
    }

    // Alternate version of the paragraph above, which tests how much of the
    // data was lost. Useful for testing how quickly the SRAM chip loses data.
    // switchToReadMode();
    // uint16_t numMessedUp = 0;
    // for (uint16_t address = 0; address < testLength; address++) {
    //     if (readByte(address) != 0x22) {
    //         numMessedUp++;
    //     }
    // }
    // Serial.write(static_cast<uint8_t>(numMessedUp >> 8));
    // Serial.write(static_cast<uint8_t>(numMessedUp));
    // bool isNonVolatile = !static_cast<bool>(numMessedUp);

    // It's like we were never there.
    switchToWriteMode();
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
