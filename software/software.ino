#include <Bounce2.h>
#include "channelio.hpp"
#include "memorychip.hpp"

// If you want to use an MCU or pinout other than the ones found in the
// hardware directory, change these settings here to your liking. 
#if !defined(PCB_LAYOUT) && (defined(BREADBOARD_LAYOUT) || defined(ARDUINO_AVR_NANO))

// Memory chip control pins
#define PIN_MEMORY_POWER A3
#define PIN_MEMORY_POWER_ACTIVE_STATE HIGH
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

MemoryChip memoryChip(&ADDRESS_CHANNEL, &DATA_CHANNEL,
                      PIN_MEMORY_CE, PIN_MEMORY_OE, PIN_MEMORY_WE,
                      PIN_MEMORY_POWER, PIN_MEMORY_POWER_ACTIVE_STATE);

Bounce testButton = Bounce();

uint16_t addresses[] = {0x0000, 0x0123, 0x0ABC, 0x0DEF};
uint8_t testBytes[] = {0x01, 0x02, 0x03, 0x04};
size_t numTests = sizeof(addresses) / sizeof(*addresses);

void printPaddedHex(unsigned int n, unsigned int digits)
{
    unsigned int nCheck = n;
    do {
        if (digits) {
            digits--;
        } else {
            break;
        }
        nCheck >>= 4;
    } while (nCheck);
    for (unsigned int i = 0; i < digits; i++) {
        Serial.print("0");
    }
    Serial.print(n, HEX);
}

void testWrite(uint16_t address, uint8_t data)
{
    Serial.print("Write to 0x");
    printPaddedHex(address, 4);
    Serial.print(": 0x");
    printPaddedHex(data, 2);
    Serial.println();
    memoryChip.writeByte(address, data);
}

uint8_t testRead(const char* messageStart, uint16_t address)
{
    uint8_t data = memoryChip.readByte(address);
    Serial.print(messageStart);
    printPaddedHex(address, 4);
    Serial.print(": 0x");
    printPaddedHex(data, 2);
    Serial.println();
    return data;
}

uint8_t testRead(uint16_t address)
{
    return testRead("Read from 0x", address);
}

void setup()
{
    testButton.attach(PIN_TEST_BUTTON, INPUT_PULLUP);
    testButton.interval(25);
    pinMode(PIN_HAPPY_LED, OUTPUT);
    pinMode(PIN_FROWNY_LED, OUTPUT);

    Serial.begin(9600);
    Serial.println("Starting!");
    memoryChip.initPins();
}

#define UNKNOWN_STATE 0
#define ALWAYS_GOOD 1
#define BAD_JUST_NOW 2
#define PREVIOUSLY_BAD 3

int prevState = UNKNOWN_STATE;
int curState = UNKNOWN_STATE;

bool buttonHeld = false;
unsigned long int buttonHeldStart = 0;
bool blinkState = LOW;
unsigned long int lastBlink = 0;
bool doWriteThisButtonPress = false;

void updateLeds(int state)
{
    switch (state) {
        case UNKNOWN_STATE:
            digitalWrite(PIN_HAPPY_LED, LOW);
            digitalWrite(PIN_FROWNY_LED, LOW);
            break;
        case ALWAYS_GOOD:
            digitalWrite(PIN_HAPPY_LED, HIGH);
            digitalWrite(PIN_FROWNY_LED, LOW);
            break;
        case BAD_JUST_NOW:
            digitalWrite(PIN_HAPPY_LED, LOW);
            digitalWrite(PIN_FROWNY_LED, HIGH);
            break;
        case PREVIOUSLY_BAD:
            digitalWrite(PIN_HAPPY_LED, HIGH);
            digitalWrite(PIN_FROWNY_LED, HIGH);
            break;
    }
}

void loop()
{
    unsigned long int curMillis = millis();

    testButton.update();
    if (testButton.fell()) {
        digitalWrite(PIN_HAPPY_LED, LOW);
        digitalWrite(PIN_FROWNY_LED, LOW);
        buttonHeld = true;
        buttonHeldStart = curMillis;
    }
    if (!testButton.rose()) {
        if (buttonHeld && curMillis - buttonHeldStart >= 1000) {
            if (!doWriteThisButtonPress || curMillis - lastBlink >= 500) {
                blinkState = !blinkState;
                digitalWrite(PIN_HAPPY_LED, blinkState);
                digitalWrite(PIN_FROWNY_LED, blinkState);
                lastBlink = curMillis;
            }
            doWriteThisButtonPress = true;
        }
        return;
    }
    buttonHeld = false;
    blinkState = LOW;

    if (doWriteThisButtonPress) {
        memoryChip.switchToWriteMode();
        for (size_t i = 0; i < numTests; i++) {
            testWrite(addresses[i], testBytes[i]);
        }
        memoryChip.switchToReadMode();
        doWriteThisButtonPress = false;
        curState = UNKNOWN_STATE;
    } else {
        bool allOnTheUpAndUp = true;
        for (size_t i = 0; i < numTests; i++) {
            uint8_t readByte = testRead(addresses[i]);
            if (readByte != testBytes[i]) {
                allOnTheUpAndUp = false;
            }
        }
        
        if (!allOnTheUpAndUp) {
            curState = BAD_JUST_NOW;
        } else if (prevState != BAD_JUST_NOW && prevState != PREVIOUSLY_BAD) {
            curState = ALWAYS_GOOD;
        } else {
            curState = PREVIOUSLY_BAD;
        }

    }

    prevState = curState;
    updateLeds(curState);
}
