#include "fastpins.hpp"

#include <stdint.h>

PinPortInfo pinToPortInfo(uint8_t pin)
{
    // These utility functions are from the Arduino core,
    // and are what's used in pinMode et. al.
    uint8_t port = digitalPinToPort(pin);
    uint8_t bitMask = digitalPinToBitMask(pin);
    uint8_t bitNum = 0;
    uint8_t bitMaskCheck = bitMask;
    while (bitMaskCheck != 1) {
        bitNum++;
        bitMaskCheck >>= 1;
    }
    PinPortInfo portInfo = {
        pin,
        portInputRegister(port),
        portOutputRegister(port),
        portModeRegister(port),
        bitNum,
        bitMask
    };
    return portInfo;
}
