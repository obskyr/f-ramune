#include <stdint.h>
#include "Arduino.h"

#ifdef INCLUDING_CHANNELIO_TEMPLATES
// Template implementations.

#include "fastpins.hpp"

template <class OutputType, class Contained>
InputChannelSet<OutputType, Contained>::InputChannelSet(
    unsigned int numChannels, Contained* channels[]) :
    _numChannels(numChannels), _inputChannels(channels) {}

template <class OutputType, class Contained>
OutputType InputChannelSet<OutputType, Contained>::input()
{
    OutputType n = 0;
    for (unsigned int i = 0; i < _numChannels; i++) {
        // Addition here will work the same as a binary or if the
        // channels only return non-overlapping bits.
        n += _inputChannels[i]->input();
    }
    return n;
}

template <class OutputType, class Contained>
void InputChannelSet<OutputType, Contained>::initInput()
{
    for (unsigned int i = 0; i < _numChannels; i++) {
        _inputChannels[i]->initInput();
    }
}

template <class OutputType, class Contained>
OutputChannelSet<OutputType, Contained>::OutputChannelSet(
    unsigned int numChannels, Contained* channels[]) :
    _numChannels(numChannels), _outputChannels(channels) {}

template <class OutputType, class Contained>
void OutputChannelSet<OutputType, Contained>::output(OutputType n)
{
    for (unsigned int i = 0; i < _numChannels; i++) {
        _outputChannels[i]->output(n);
    }
}

template <class OutputType, class Contained>
void OutputChannelSet<OutputType, Contained>::initOutput()
{
    for (unsigned int i = 0; i < _numChannels; i++) {
        _outputChannels[i]->initOutput();
    }
}

template <class OutputType, class Contained>
InputOutputChannelSet<OutputType, Contained>::InputOutputChannelSet(
    unsigned int numChannels, Contained* channels[]) :
    InputChannelSet<OutputType, Contained>(numChannels, channels),
    OutputChannelSet<OutputType, Contained>(numChannels, channels),
    _numChannels(numChannels), _channels(channels) {}

template <class T>
Output_ShiftRegister<T>::Output_ShiftRegister(
    unsigned int dataPin, unsigned int shiftPin,
    unsigned int latchPin, unsigned int numBits) :
    _dataPin(pinToPortInfo(dataPin)), _shiftPin(pinToPortInfo(shiftPin)),
    _latchPin(pinToPortInfo(latchPin)), _numBits(numBits) {}

template <class T>
void Output_ShiftRegister<T>::output(T n)
{
    SET_BITS_IN_PORT_LOW(_latchPin.out, _latchPin.bitMask);

    for (int bitNum = _numBits - 1; bitNum >= 0; bitNum--) {
        SET_BIT_IN_PORT(_dataPin.out, _dataPin.bitNum, (n >> bitNum) & 1);
        SET_BITS_IN_PORT_HIGH(_shiftPin.out, _shiftPin.bitMask);
        SET_BITS_IN_PORT_LOW(_shiftPin.out, _shiftPin.bitMask);
    }

    SET_BITS_IN_PORT_HIGH(_latchPin.out, _latchPin.bitMask);
}

template <class T>
void Output_ShiftRegister<T>::initOutput()
{
    pinMode(_dataPin.pin, OUTPUT);
    pinMode(_shiftPin.pin, OUTPUT);
    pinMode(_latchPin.pin, OUTPUT);
}

#else
// Non-template implementations.
#include "channelio.hpp"

InputOutput_Port::InputOutput_Port(
    volatile uint8_t* inputRegister,
    volatile uint8_t* outputRegister,
    volatile uint8_t* directionRegister,
    unsigned int portStartBit,
    unsigned int valueStartBit,
    unsigned int numBits) :
    _inputRegister(inputRegister), _outputRegister(outputRegister),
    _directionRegister(directionRegister), _portStartBit(portStartBit),
    _valueStartBit(valueStartBit), _numBits(numBits)
{
    for (unsigned int bitNum = 0; bitNum < _numBits; bitNum++) {
        _portMask |= 1 << (_portStartBit + bitNum);
    }
}

uint8_t InputOutput_Port::input()
{
    uint8_t n = *_inputRegister & _portMask;
    n >>= _portStartBit;
    n <<= _valueStartBit;
    return n;
}

void InputOutput_Port::output(uint8_t n)
{
    Serial.println("Called InputOutput_Port::output.");
    n >>= _valueStartBit;
    n <<= _portStartBit;
    n &= _portMask;
    *_outputRegister = (*_outputRegister & ~_portMask) | n;
}

void InputOutput_Port::initInput()
{
    *_directionRegister &= ~_portMask;
    *_outputRegister &= ~_portMask; // Turns off INPUT_PULLUP.
}

void InputOutput_Port::initOutput()
{
    *_directionRegister |= _portMask;
}

#endif
