#ifndef CHANNELIO_HPP
#define CHANNELIO_HPP

#include <stdint.h>

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
    InputChannelSet(unsigned int numChannels, const InputChannel<T>* channels[]);
    T input();
    void initInput();
private:
    int _numChannels;
    InputChannel<T>** _channels;
};

template <class T>
class OutputChannelSet : virtual public OutputChannel<T>
{
public:
    OutputChannelSet(unsigned int numChannels, OutputChannel<T>* channels[]);
    void output(T n);
    void initOutput();
private:
    int _numChannels;
    OutputChannel<T>** _channels;
};

template <class T>
class InputOutputChannelSet : public InputChannelSet<T>,
                              public OutputChannelSet<T>,
                              public InputOutputChannel<T>
{
public:
    InputOutputChannelSet(unsigned int numChannels, InputOutputChannel<T>* channels[]);
private:
    int _numChannels;
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
    unsigned int _dataPin;
    unsigned int _shiftPin;
    unsigned int _latchPin;
    unsigned int _numBits;
};

class InputOutput_Port : public InputOutputChannel<uint8_t>
{
public:
    InputOutput_Port(uint8_t* inputRegister, uint8_t* outputRegister,
                     uint8_t* directionRegister, unsigned int portStartBit,
                     unsigned int valueStartBit, unsigned int numBits);
    uint8_t input();
    void output(uint8_t n);
    void initInput();
    void initOutput();
private:
    uint8_t* _inputRegister;
    uint8_t* _outputRegister;
    uint8_t* _directionRegister;
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
