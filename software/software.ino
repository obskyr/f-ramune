#include "channelio.hpp"
#include "memorychip.hpp"

// If you want to use an MCU or pinout than the ones found in the
// hardware directory, change these settings here to your liking. 
#if !defined(PCB_LAYOUT) && (defined(BREADBOARD_LAYOUT) || defined(ARDUINO_AVR_NANO))

// Memory chip control pins
#define PIN_MEMORY_POWER A3
#define PIN_MEMORY_CE    A4
#define PIN_MEMORY_OE    A5
#define PIN_MEMORY_WE    2

// Memory chip address channel
const Output_ShiftRegister<uint16_t> ADDRESS_CHANNEL(
     8, // Bit on pin 8,
    10, // shift on pin 10,
     9, // latch on pin 9,
    15  // and 15 bits.
);

// Memory chip data channel
// 5 bits starting at bit #3, via port D
const InputOutput_Port DATA_CHANNEL_PORT_1(&PIND, &PORTD, &DDRD, 3, 3, 5);
// 3 bits starting at bit #0, via port C
const InputOutput_Port DATA_CHANNEL_PORT_2(&PINC, &PORTC, &DDRC, 0, 0, 3);
const InputOutput_Port* DATA_CHANNEL_PORTS[] = {
    &DATA_CHANNEL_PORT_1,
    &DATA_CHANNEL_PORT_2
};
const InputOutputChannelSet<uint8_t> DATA_CHANNEL(
    sizeof(DATA_CHANNEL_PORTS) / sizeof(*DATA_CHANNEL_PORTS),
    DATA_CHANNEL_PORTS
);

// Buttons 'n' lights
#define PIN_TEST_BUTTON 13
#define PIN_HAPPY_LED   12
#define PIN_FROWNY_LED  11

#else

// I decree that the IO settings for the circuit board version shall go here.

#endif

MemoryChip memoryChip(&ADDRESS_CHANNEL, &DATA_CHANNEL,
                      PIN_MEMORY_CE, PIN_MEMORY_OE,
                      PIN_MEMORY_WE, PIN_MEMORY_POWER);

void setup() {
}

void loop() {
}
