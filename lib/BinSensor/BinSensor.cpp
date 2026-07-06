#include "BinSensor.h"

bool BinSensor::begin(
    uint8_t trigPin,
    uint8_t echoPin)
{
    trig = trigPin;
    echo = echoPin;

    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);

    digitalWrite(trig, LOW);

    return true;
}

float BinSensor::distance()
{
    digitalWrite(trig, LOW);
    delayMicroseconds(2);

    digitalWrite(trig, HIGH);
    delayMicroseconds(10);

    digitalWrite(trig, LOW);

    long duration = pulseIn(
        echo,
        HIGH,
        25000UL);

    if(duration == 0)
        return 999;

    return duration * 0.0343f / 2.0f;
}

bool BinSensor::isFull()
{
    return distance() <= FULL_DISTANCE;
}