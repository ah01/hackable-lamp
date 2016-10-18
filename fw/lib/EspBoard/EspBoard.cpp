#include "EspBoard.h"

void EspBoardClass::begin()
{
    // led
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
};

void EspBoardClass::color(byte red, byte green, byte blue)
{
    analogWrite(LED_RED_PIN, 1024 - (red * 4));
    analogWrite(LED_GREEN_PIN, 1024 - (green * 4));
    analogWrite(LED_BLUE_PIN, 1024 - (blue * 4));
};
