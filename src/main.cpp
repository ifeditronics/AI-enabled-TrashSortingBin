#include <Arduino.h>

#include "Classifier.h"
#include "ServoControl.h"
#include "BinSensor.h"

Classifier classifier;
ServoControl servo;
BinSensor bin;

/************************************************
 * Hardware Pin Definitions
 ***********************************************/

#define TRIG_PIN    13
#define ECHO_PIN    15

#define SERVO_PIN   14

#define GREEN_LED   2
#define RED_LED     12

/************************************************/

void setup()
{
    Serial.begin(115200);

    Serial.println();
    Serial.println("==============================");
    Serial.println(" SMART AI TRASH BIN");
    Serial.println("==============================");

    if (!classifier.begin())
    {
        Serial.println("Classifier initialization failed!");

        while (true)
        {
            delay(1000);
        }
    }

    servo.begin(
        SERVO_PIN,
        GREEN_LED,
        RED_LED);

    bin.begin(
        TRIG_PIN,
        ECHO_PIN);

    servo.home();

    Serial.println("System Ready.");
}

void loop()
{
    WasteType waste = classifier.classify();

    switch (waste)
    {
        case RECYCLABLE:

            Serial.println("Recyclable");

            servo.recyclable();

            delay(1500);

            servo.home();

            break;

        case NON_RECYCLABLE:

            Serial.println("Non Recyclable");

            servo.nonRecyclable();

            delay(1500);

            // Only check bin fullness
            // when pointing to the non-recyclable side
            if (bin.isFull())
            {
                Serial.println("WARNING: BIN FULL!");

                servo.warning();
            }

            servo.home();

            break;

        default:

            Serial.println("Unknown");

            servo.home();

            break;
    }

    delay(500);
}