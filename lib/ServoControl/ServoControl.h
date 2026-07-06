#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoControl
{
public:

    bool begin(
        uint8_t servoPin,
        uint8_t greenLedPin,
        uint8_t redLedPin);

    void recyclable();

    void nonRecyclable();

    void home();

    void unknown();

    void warning();

    bool isOnNonRecyclableSide();

private:

    Servo servo;

    uint8_t servoPin;
    uint8_t greenLED;
    uint8_t redLED;

    bool nonRecyclablePosition = false;

    static constexpr uint8_t HOME = 90;
    static constexpr uint8_t RECYCLABLE = 20;
    static constexpr uint8_t NON_RECYCLABLE = 150;

    void ledsOff();
};