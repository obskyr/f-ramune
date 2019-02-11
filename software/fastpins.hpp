#ifndef FASTPINS_HPP
#define FASTPINS_HPP

#include <stdint.h>
#include <Arduino.h>

struct PinPortInfo
{
    uint8_t pin;
    volatile uint8_t* in;
    volatile uint8_t* out;
    volatile uint8_t* direction;
    uint8_t bitNum;
    uint8_t bitMask;
};

PinPortInfo pinToPortInfo(uint8_t pin);

#define READ_BIT_IN_PORT(inputReg, bitNum) ((*(inputReg) >> (bitNum)) & 1)
#define READ_BITS_IN_PORT(inputReg, bitMask) (*(inputReg) & (bitMask))
#define SET_BIT_IN_PORT(outputReg, bitNum, val) \
    (*(outputReg) = (*(outputReg) & ~(1 << bitNum)) | ((val) << (bitNum)))
#define SET_BITS_IN_PORT_HIGH(outputReg, bitMask) (*(outputReg) |= (bitMask))
#define SET_BITS_IN_PORT_LOW(outputReg, bitMask) (*(outputReg) &= ~(bitMask))

#endif
