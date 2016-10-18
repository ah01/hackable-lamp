#ifndef ESP_BOARD_H
#define ESP_BOARD_H

#include "Arduino.h"

// define HW pin
#define LED_RED_PIN 14
#define LED_GREEN_PIN 12
#define LED_BLUE_PIN 16

class EspBoardClass
{
    public:
        void begin();
        void color(byte red, byte green, byte blue);
};

// instance
//extern EspBoardClass EspBoard;

#endif
