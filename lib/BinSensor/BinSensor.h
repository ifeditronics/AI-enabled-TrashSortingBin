#pragma once

#include <Arduino.h>

class BinSensor
{
public:

    bool begin(
        uint8_t trigPin,
        uint8_t echoPin);

    float distance();

    bool isFull();

private:

    uint8_t trig;
    uint8_t echo;

    static constexpr float FULL_DISTANCE = 7.0f;
};