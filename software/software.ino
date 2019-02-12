#include "channelio.hpp"
#include "memorychip.hpp"

// If you want to use an MCU or pinout other than the ones found in the
// hardware directory, change these settings here to your liking. 
#if !defined(PCB_LAYOUT) && (defined(BREADBOARD_LAYOUT) || defined(ARDUINO_AVR_NANO))

// Memory chip control pins
#define PIN_MEMORY_POWER A3
#define PIN_MEMORY_CE    A4
#define PIN_MEMORY_OE    A5
#define PIN_MEMORY_WE    2

// Memory chip address channel
// Latch pin: 10 (SS - doesn't need to be SS, but might as well be)
// When SPI is active, don't forget: MISO's direction is overridden to be an
// input, and SS has to stay configured as an output to stay master!
// Within those config constraints, though, they can be used for unrelated IO.
Output_SpiShiftRegister<uint16_t> ADDRESS_CHANNEL(20000000, SPI_MODE0, 10);

// Memory chip data channel
// 5 bits starting at bit #3, via port D
InputOutput_Port DATA_CHANNEL_PORT_1(&PIND, &PORTD, &DDRD, 3, 3, 5);
// 3 bits starting at bit #0, via port C
InputOutput_Port DATA_CHANNEL_PORT_2(&PINC, &PORTC, &DDRC, 0, 0, 3);
InputOutput_Port* DATA_CHANNEL_PORTS[] = {
    &DATA_CHANNEL_PORT_1,
    &DATA_CHANNEL_PORT_2
};
InputOutputChannelSet<uint8_t, InputOutput_Port> DATA_CHANNEL(
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
    Serial.begin(9600);
    Serial.println("Starting!");
    ADDRESS_CHANNEL.initOutput();
    memoryChip.switchToWriteMode();

    unsigned long t1 = millis();
    for (unsigned int x = 0; x < 10000; x++)  {
        memoryChip.writeByte(x, 0xA5);
    }
    unsigned long t2 = millis();
    Serial.print("Microseconds per write (target: <69): ");
    Serial.println(((t2 - t1) * 1000) / 10000.0);
}

int i = 0;
uint8_t nums[] = {0b10000001, 0, 0b00000001, 0b10000000};

void loop() {
    i += 1;
    i %= sizeof(nums) / sizeof(*nums);
    ADDRESS_CHANNEL.output(nums[i]);
    delay(700);
}
