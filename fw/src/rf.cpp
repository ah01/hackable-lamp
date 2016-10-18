#include <Arduino.h>
#include "rf.h"

// --- private stuff ----------------------------------------------------------

#define RF_SHORT_PULS 1
#define RF_LONG_PULS 3
#define RF_STOP_PULS 32

/**
 * Send single pulse, block until send
 *
 * @param highT time of high state
 * @param lowT  time of low state
 */
void RfTransmitter::puls(unsigned int highT, unsigned int lowT)
{
    digitalWrite(pin, HIGH);
    delayMicroseconds(baseTime * highT);
    digitalWrite(pin, LOW);
    delayMicroseconds(baseTime * lowT);
}

/**
 * Send single synbol, block until send
 *
 * @param symbol Symbol
 */
void RfTransmitter::send_symbol(char symbol)
{
    switch(symbol)
    {
        case '0':
            puls(RF_SHORT_PULS, RF_LONG_PULS);
            puls(RF_SHORT_PULS, RF_LONG_PULS);
            break;
        case '1':
            puls(RF_LONG_PULS, RF_SHORT_PULS);
            puls(RF_LONG_PULS, RF_SHORT_PULS);
            break;
        case 'F':
            puls(RF_SHORT_PULS, RF_LONG_PULS);
            puls(RF_LONG_PULS, RF_SHORT_PULS);
            break;
        case 'S':
            puls(RF_SHORT_PULS, RF_STOP_PULS);
            break;
        default:
            return;
    }
}

/* obsolete
void rf_send_frame(const char* data, unsigned int period)
{
    int i = 0;
    while(data[i] != 0)
    {
        send_symbol(data[i], period);
        i++;
    }
}
*/

/*
void rf_send(const char* data, unsigned int period, unsigned int repeate)
{
    Serial.print("Send cmd: ");
    Serial.print(data);
    Serial.print("@");
    Serial.println(period);

    while(repeate-- > 0)
    {
        rf_send_frame(data, period);
    }

}
*/

// --- Public stuff -----------------------------------------------------------

RfTransmitter::RfTransmitter(byte txPin, unsigned int time)
{
    pin = txPin;
    baseTime = time;
}

void RfTransmitter::begin()
{
    pinMode(pin, OUTPUT);
}

void RfTransmitter::send(const char* code)
{
    int i = 0;
    while(code[i] != 0)
    {
        send_symbol(code[i]);
        i++;
    }
}
