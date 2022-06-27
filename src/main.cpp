#include <Arduino.h>
#include <PWM.hpp> //https://github.com/xkam1x/Arduino-PWM-Reader
#include <Servo.h>
#include <Adafruit_SCD30.h>
#include <SD.h>
#include <TimeLib.h>

File myFile;
Adafruit_SCD30 scd30;
Servo throttle;

PWM ch1(2); // Setup pin 2 for input
PWM ch2(3); // Setup pin 3 for input

int throttle_channel;
int arm_channel;
int throttle_output = 1500;

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void setup()
{
    setSyncProvider(getTeensy3Time);

    // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(56700);
    delay(1000);
    Serial.println("Hello there");

    ch1.begin(true); // ch1 on pin 2 reading PWM HIGH duration
    ch2.begin(true); // ch2 on pin 3 reading PWM HIGH duration
    throttle.attach(5);

    if (!scd30.begin())
    {
        Serial.println("Failed to find SCD30 chip");
    }
    else
    {
        Serial.println("SCD30 Found!");
        Serial.print("Measurement Interval: ");
        Serial.print(scd30.getMeasurementInterval());
        Serial.println(" seconds");
    }

    if (!scd30.setMeasurementInterval(2))
    {
        Serial.println("Failed to set measurement interval");
    }

    Serial.print("Initializing SD card...");

    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println("SD initialization failed!");
    }
    else
    {
        Serial.println("SD initialization done.");
    }

    myFile = SD.open("data.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (myFile)
    {
        Serial.print("test SD card write");
        myFile.println("Power on");
        Serial.println("done.");
    }
    else
    {
        // if the file didn't open, print an error:
        Serial.println("error opening data.txt");
    }
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
    float start = micros();
    throttle_channel = ch1.getValue();
    arm_channel = ch2.getValue();

    // when armed channel high, output the throttle signal
    if (arm_channel > 1500)
    {
        throttle_output = throttle_channel;
    }
    else
    {
        throttle_output = 1500;
    }
    throttle.writeMicroseconds(throttle_output);

    if (scd30.dataReady())
    {
        if (!scd30.read())
        {
            Serial.println("Error reading sensor data");
            return;
        }
    }

    myFile.printf("%f - %d:%d:%d - %f, %f, %f, %d, %d \n", micros() - start, hour(), minute(), second(), scd30.temperature, scd30.relative_humidity, scd30.CO2, throttle_channel, arm_channel);
    Serial.printf("%f - %d:%d:%d - %f, %f, %f, %d, %d \n", micros() - start, hour(), minute(), second(), scd30.temperature, scd30.relative_humidity, scd30.CO2, throttle_channel, arm_channel);
    myFile.flush();
}
