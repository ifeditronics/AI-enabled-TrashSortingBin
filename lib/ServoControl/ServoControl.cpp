#include "ServoControl.h"

bool ServoControl::begin(
    uint8_t sPin,
    uint8_t gPin,
    uint8_t rPin)
{
    servoPin = sPin;
    greenLED = gPin;
    redLED = rPin;

    pinMode(greenLED, OUTPUT);
    pinMode(redLED, OUTPUT);

    ledsOff();

    servo.setPeriodHertz(50);

    servo.attach(
        servoPin,
        500,
        2400);

    servo.write(HOME);

    nonRecyclablePosition = false;

    Serial.println("Servo Ready");

    return true;
}

void ServoControl::ledsOff()
{
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);
}

void ServoControl::home()
{
    servo.write(HOME);

    ledsOff();

    nonRecyclablePosition = false;
}

void ServoControl::recyclable()
{
    servo.write(RECYCLABLE);

    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);

    nonRecyclablePosition = false;
}

void ServoControl::nonRecyclable()
{
    servo.write(NON_RECYCLABLE);

    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);

    nonRecyclablePosition = true;
}

void ServoControl::warning()
{
    for(int i = 0; i < 8; i++)
    {
        digitalWrite(greenLED, HIGH);
        digitalWrite(redLED, HIGH);

        delay(150);

        digitalWrite(greenLED, LOW);
        digitalWrite(redLED, LOW);

        delay(150);
    }
}

void ServoControl::unknown()
{
    servo.write(HOME);

    nonRecyclablePosition = false;

    for(int i = 0; i < 3; i++)
    {
        digitalWrite(greenLED, HIGH);
        digitalWrite(redLED, HIGH);

        delay(120);

        digitalWrite(greenLED, LOW);
        digitalWrite(redLED, LOW);

        delay(120);
    }
}

bool ServoControl::isOnNonRecyclableSide()
{
    return nonRecyclablePosition;
}