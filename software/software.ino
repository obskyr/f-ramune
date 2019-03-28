#include <Bounce2.h>
#include "channelio.hpp"
#include "memorychip.hpp"
#include "serialinterface.hpp"

// If you want to use an MCU or pinout other than the ones found in the
// hardware directory, change these settings here to your liking. 
#if !defined(PCB_LAYOUT) && (defined(BREADBOARD_LAYOUT) || defined(ARDUINO_AVR_NANO))

// Memory chip control pins
#define PIN_MEMORY_POWER A3
#define PIN_MEMORY_POWER_ON_STATE HIGH
#define PIN_MEMORY_CE    A4
#define PIN_MEMORY_OE    A5
#define PIN_MEMORY_WE    2

// Memory chip address channel
// Latch pin: 10 (SS - doesn't need to be SS, but might as well be)
// When SPI is active, don't forget: MISO's direction is overridden
// to be an input (but the pull-up is still available), and SS has to
// stay configured as an output to stay master!
// Within those config constraints, though, they can be used for unrelated IO.
Output_SpiShiftRegister<uint16_t> ADDRESS_CHANNEL(20000000, SPI_MODE0, 10);

// Memory chip data channel
// Note that INPUT_PULLUP here is important - otherwise, reading when there's
// no chip connected will read back what was just written (ghooost values).
// 5 bits starting at bit #3, via port D
InputOutput_Port DATA_CHANNEL_PORT_1(INPUT_PULLUP, &PIND, &PORTD, &DDRD, 3, 3, 5);
// 3 bits starting at bit #0, via port C
InputOutput_Port DATA_CHANNEL_PORT_2(INPUT_PULLUP, &PINC, &PORTC, &DDRC, 0, 0, 3);
InputOutput_Port* DATA_CHANNEL_PORTS[] = {
    &DATA_CHANNEL_PORT_1,
    &DATA_CHANNEL_PORT_2
};
InputOutputChannelSet<uint8_t, InputOutput_Port> DATA_CHANNEL(
    sizeof(DATA_CHANNEL_PORTS) / sizeof(*DATA_CHANNEL_PORTS),
    DATA_CHANNEL_PORTS
);

// Buttons 'n' lights
#define PIN_TEST_BUTTON 12
#define PIN_HAPPY_LED    9
#define PIN_FROWNY_LED   8

#else

// I decree that the IO settings for the circuit board version shall go here.

#endif

MemoryChip MEMORY_CHIP(&ADDRESS_CHANNEL, &DATA_CHANNEL,
                       PIN_MEMORY_CE, PIN_MEMORY_OE, PIN_MEMORY_WE,
                       PIN_MEMORY_POWER, PIN_MEMORY_POWER_ON_STATE);
SerialInterface SERIAL_INTERFACE(&Serial, &MEMORY_CHIP);

Bounce TEST_BUTTON = Bounce();

bool testChip()
{
    MemoryChipKnownProperties prevKnownProperties;
    MemoryChipProperties prevProperties;
    MEMORY_CHIP.getProperties(&prevKnownProperties, &prevProperties);
    MEMORY_CHIP.analyze();
    MemoryChipKnownProperties knownProperties;
    MemoryChipProperties properties;
    MEMORY_CHIP.getProperties(&knownProperties, &properties);
    MEMORY_CHIP.setProperties(&prevKnownProperties, &prevProperties);

    // To change what the test button tests, change these here criteria!
    // These criteria test that the chip is fast 32 KiB non-volatile memory.
    // That is, FRAM. (Or something equivalent, but it's gonna be FRAM.)
    return (knownProperties.isOperational && properties.isOperational) &&
           (knownProperties.size && properties.size == 0x8000) &&
           (knownProperties.isNonVolatile && properties.isNonVolatile) &&
           (knownProperties.isSlow && !properties.isSlow);
}

void setup()
{
    Serial.begin(115200);
    MEMORY_CHIP.initPins();
    TEST_BUTTON.attach(PIN_TEST_BUTTON, INPUT_PULLUP);
    TEST_BUTTON.interval(25);
    pinMode(PIN_HAPPY_LED, OUTPUT);
    pinMode(PIN_FROWNY_LED, OUTPUT);
}

void loop()
{
    TEST_BUTTON.update();
    if (!SERIAL_INTERFACE.update()) {
        if (TEST_BUTTON.fell()) {
            digitalWrite(PIN_HAPPY_LED, LOW);
            digitalWrite(PIN_FROWNY_LED, LOW);
        } else if (TEST_BUTTON.rose()) {
            MEMORY_CHIP.powerOn();
            if (testChip()) {
                digitalWrite(PIN_HAPPY_LED, HIGH);
            } else {
                digitalWrite(PIN_FROWNY_LED, HIGH);
            }
            MEMORY_CHIP.powerOff();
        }
    }
}
