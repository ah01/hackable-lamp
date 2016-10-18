#ifndef RF_H
#define RF_H

#include <Arduino.h>

class RfTransmitter
{
    public:
        RfTransmitter(byte pin, unsigned int baseTime);
        void begin();
        void send(const char* code);
    private:
        byte pin;
        unsigned int baseTime;
        void puls(unsigned int highT, unsigned int lowT);
        void send_symbol(char symbol);
};

#endif
