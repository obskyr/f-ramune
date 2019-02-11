#ifndef CHANNELIO_HPP
#define CHANNELIO_HPP

#include <stdint.h>
#include "fastpins.hpp"

// Base classes

template <class T>
class InputChannel
{
public:
    virtual ~InputChannel() {}
    virtual T input() = 0;
    virtual void initInput() = 0;
};

template <class T>
class OutputChannel
{
public:
    virtual ~OutputChannel() {}
    virtual void output(T n) = 0;
    virtual void initOutput() = 0;
};

template <class T>
class InputOutputChannel : virtual public InputChannel<T>,
                           virtual public OutputChannel<T>
{
public:
    virtual ~InputOutputChannel() {}
};

// Channel sets (multiple channels triggered via a single object)

template <class T>
class InputChannelSet : virtual public InputChannel<T>
{
public:
    InputChannelSet(unsigned int numChannels,
                    InputChannel<T>* channels[]);
    T input();
    void initInput();
private:
    unsigned int _numChannels;
    InputChannel<T>** _inputChannels;
};

template <class T>
class OutputChannelSet : virtual public OutputChannel<T>
{
public:
    OutputChannelSet(unsigned int numChannels,
                     OutputChannel<T>* channels[]);
    virtual void output(T n);
    void initOutput();
private:
    unsigned int _numChannels;
    OutputChannel<T>** _outputChannels;
};

template <class T>
class InputOutputChannelSet : virtual public InputChannelSet<T>,
                              virtual public OutputChannelSet<T>,
                              virtual public InputOutputChannel<T>
{
public:
    InputOutputChannelSet(unsigned int numChannels,
                          InputOutputChannel<T>* channels[]);

    // TODO: This isn't an actual solution - sure, upcasting to
    // OutputChannelSet works now, but try initializing an OutputChannelSet
    // with an array of InputOutputChannels and reality breaks down around you.
    void output(T n) {
        for (unsigned int i = 0; i < _numChannels; i++) {
            _channels[i]->output(n);
        }
    }
    
private:
    unsigned int _numChannels;
    InputOutputChannel<T>** _channels;
};

// Specific IO types

template <class T>
class Output_ShiftRegister : public OutputChannel<T>
{
public:
    Output_ShiftRegister(unsigned int dataPin, unsigned int shiftPin,
                         unsigned int latchPin, unsigned int numBits);
    void output(T n);
    void initOutput();
private:
    PinPortInfo _dataPin;
    PinPortInfo _shiftPin;
    PinPortInfo _latchPin;
    unsigned int _numBits;
};

class InputOutput_Port : public InputOutputChannel<uint8_t>
{
public:
    InputOutput_Port(volatile uint8_t* inputRegister,
                     volatile uint8_t* outputRegister,
                     volatile uint8_t* directionRegister,
                     unsigned int portStartBit,
                     unsigned int valueStartBit,
                     unsigned int numBits);
    uint8_t input();
    void output(uint8_t n);
    void initInput();
    void initOutput();
private:
    volatile uint8_t* _inputRegister;
    volatile uint8_t* _outputRegister;
    volatile uint8_t* _directionRegister;
    unsigned int _portStartBit;
    unsigned int _valueStartBit;
    unsigned int _numBits;
    uint8_t _portMask;
};

// This very, very strange way of separating declaration from implementation
// was wrought upon this world not by my hand, but by a combination of C++'s
// questionable handling of templates and Arduino's questionable rigidity
// in code structure and file extensions. Bon appetit.
#define INCLUDING_CHANNELIO_TEMPLATES
#include "channelio.cpp"
#undef INCLUDING_CHANNELIO_TEMPLATES

#endif
