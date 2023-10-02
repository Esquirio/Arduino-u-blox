#include "Arduino.h"
#include <DHT11.h>

// Create an instance of the DHT11 class.
// - For NINA-W10 (ESP32): Connect the sensor to pin GPIO26 or 26.
#define DIGITAL_PORT 26
DHT11 dht11(DIGITAL_PORT);

// Declare Global variables
int8_t temp = NAN;
int8_t humi = NAN;

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Attempt to read the temperature and humidity values from the DHT11 sensor.
  temp = dht11.readTemperature();
  delay(50); // Recommended for ESP32 MCUs
  humi = dht11.readHumidity();

  // Check the results of the readings.
    // If there are no errors, print the temperature and humidity values in CSV format.
  if (temp != DHT11::ERROR_CHECKSUM && temp != DHT11::ERROR_TIMEOUT || 
      humi != DHT11::ERROR_CHECKSUM && humi != DHT11::ERROR_TIMEOUT)
  {
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print("ºC");
    Serial.print(", Humidity: ");
    Serial.print(humi);
    Serial.println("%");
  }
  else
    Serial.println("Failed to read temperature and humidity");
}
