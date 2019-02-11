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
Output_ShiftRegister<uint16_t> ADDRESS_CHANNEL(
     8, // Bit on pin 8,
    10, // shift on pin 10,
     9, // latch on pin 9,
    15  // and 15 bits.
);

// Memory chip data channel
// 5 bits starting at bit #3, via port D
InputOutput_Port DATA_CHANNEL_PORT_1(&PIND, &PORTD, &DDRD, 3, 3, 5);
// 3 bits starting at bit #0, via port C
InputOutput_Port DATA_CHANNEL_PORT_2(&PINC, &PORTC, &DDRC, 0, 0, 3);
InputOutput_Port* DATA_CHANNEL_PORTS[] = {
    &DATA_CHANNEL_PORT_1,
    &DATA_CHANNEL_PORT_2
};
InputOutputChannelSet<uint8_t> DATA_CHANNEL(
    sizeof(DATA_CHANNEL_PORTS) / sizeof(*DATA_CHANNEL_PORTS),
    (InputOutputChannel<uint8_t>**) DATA_CHANNEL_PORTS
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

InputOutputChannelSet<uint8_t> iocs(
    sizeof(DATA_CHANNEL_PORTS) / sizeof(*DATA_CHANNEL_PORTS),
    (InputOutputChannel<uint8_t>**) DATA_CHANNEL_PORTS
);
OutputChannelSet<uint8_t> ocs(
    sizeof(DATA_CHANNEL_PORTS) / sizeof(*DATA_CHANNEL_PORTS),
    (OutputChannel<uint8_t>**) DATA_CHANNEL_PORTS
);

void setup() {
    Serial.begin(9600);
    Serial.println("Starting!");

    Serial.println("Output directly from IOCS array:");
    ((InputOutputChannel<uint8_t>**) DATA_CHANNEL_PORTS)[0]->output(0xA0);
    Serial.println("Output directly from IOCS array cast to OCS array:");
    ((OutputChannel<uint8_t>**) DATA_CHANNEL_PORTS)[0]->output(0xA1);
    Serial.println("InputOutputChannelSet (containing IOC):");
    iocs.output(0xA5);
    Serial.println("Cast to OutputChannel:");
    ((OutputChannelSet<uint8_t>*) &iocs)->output(0xA6);
    Serial.println("OutputChannel (containing IOC):");
    ocs.output(0xA7);
    Serial.println("Start test finished.");

    ADDRESS_CHANNEL.initOutput();

    memoryChip.switchToWriteMode();
}

int i = 0;
uint8_t nums[] = {0b10000001, 0, 0b00000001, 0b10000000};

void loop() {
    i += 1;
    i %= sizeof(nums) / sizeof(*nums);
    unsigned long t1 = millis();
    for (unsigned int x = 0; x < 10000; x++)  {
        memoryChip.writeByte(x, 0xA5);
    }
    unsigned long t2 = millis();
    Serial.print("Microseconds per write (target: <69): ");
    Serial.println(((t2 - t1) * 1000) / 10000.0);
    delay(700);
}
