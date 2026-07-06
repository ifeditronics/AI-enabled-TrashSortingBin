#include <Arduino.h>

#define FLASH_LED 4

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println();
    Serial.println("================================");
    Serial.println("ESP32-CAM BLINK TEST");
    Serial.println("================================");

    pinMode(FLASH_LED, OUTPUT);

    digitalWrite(FLASH_LED, LOW);
}

void loop()
{
    Serial.println("LED ON");
    digitalWrite(FLASH_LED, HIGH);
    delay(1000);

    Serial.println("LED OFF");
    digitalWrite(FLASH_LED, LOW);
    delay(1000);
}