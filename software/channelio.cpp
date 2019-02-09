#include "Arduino.h"

#ifdef INCLUDING_CHANNELIO_TEMPLATES
// Template implementations.

template <class T>
InputChannelSet<T>::InputChannelSet(
    unsigned int numChannels, const InputChannel<T>* channels[]) :
    _numChannels(numChannels), _channels(channels) {}

template <class T>
T InputChannelSet<T>::input()
{
    T n = 0;
    for (unsigned int i = 0; i < _numChannels; i++) {
        // Addition here will work the same as a binary or if the
        // channels only return non-overlapping bits.
        n += _channels[i]->input();
    }
}

template <class T>
void InputChannelSet<T>::initInput()
{
    for (unsigned int i = 0; i < _numChannels; i++) {
        _channels[i]->initInput();
    }
}

template <class T>
OutputChannelSet<T>::OutputChannelSet(
    unsigned int numChannels, OutputChannel<T>* channels[]) :
    _numChannels(numChannels), _channels(channels) {}

template <class T>
void OutputChannelSet<T>::output(T n)
{
    for (unsigned int i = 0; i < _numChannels; i++) {
        // Addition here will work the same as a binary or if the
        // channels only return non-overlapping bits.
        _channels[i]->output(n);
    }
}

template <class T>
void OutputChannelSet<T>::initOutput()
{
    for (unsigned int i = 0; i < _numChannels; i++) {
        _channels[i]->initOutput();
    }
}

template <class T>
InputOutputChannelSet<T>::InputOutputChannelSet(
    unsigned int numChannels, InputOutputChannel<T>* channels[]) :
    // Not sure this is the best way to do this - this sets a "channels"
    // member thrice - but eh meh. It's not gonna run in a tight loop.
    InputChannelSet<T>(numChannels, channels),
    OutputChannelSet<T>(numChannels, channels),
    _numChannels(numChannels), _channels(channels) {}

template <class T>
Output_ShiftRegister<T>::Output_ShiftRegister(
    unsigned int dataPin, unsigned int shiftPin,
    unsigned int latchPin, unsigned int numBits) :
    _dataPin(dataPin), _shiftPin(shiftPin), _latchPin(latchPin),
    _numBits(numBits) {}

template <class T>
void Output_ShiftRegister<T>::output(T n)
{
    for (int bitNum = _numBits - 1; bitNum >= 0; bitNum--) {
        digitalWrite(_dataPin, (n >> bitNum) & 1);
        digitalWrite(_shiftPin, HIGH);
        digitalWrite(_shiftPin, LOW);
    }
    digitalWrite(_latchPin, HIGH);
    digitalWrite(_latchPin, LOW);
}

template <class T>
void Output_ShiftRegister<T>::initOutput()
{
    pinMode(_dataPin, OUTPUT);
    pinMode(_shiftPin, OUTPUT);
    pinMode(_latchPin, OUTPUT);
}

#else
// Non-template implementations.
#include "channelio.hpp"

InputOutput_Port::InputOutput_Port(
    uint8_t* inputRegister, uint8_t* outputRegister,
    uint8_t* directionRegister, unsigned int portStartBit,
    unsigned int valueStartBit, unsigned int numBits) :
    _inputRegister(_inputRegister), _outputRegister(outputRegister),
    _directionRegister(directionRegister), _portStartBit(portStartBit),
    _valueStartBit(valueStartBit), _numBits(numBits)
{
    for (int bitNum = 0; bitNum < _numBits; bitNum++) {
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
    n >>= _valueStartBit;
    n <<= _portStartBit;
    n &= _portMask;
    *_outputRegister = (*_outputRegister & ~_portMask) | n;
}

void InputOutput_Port::initInput()
{
    *_directionRegister &= ~_portMask;
}

void InputOutput_Port::initOutput()
{
    *_directionRegister |= _portMask;
}

#endif
