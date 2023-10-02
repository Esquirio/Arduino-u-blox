/******************************************************************************

u-blox NINA-W10 Arduino example
Fernando Esquirio Torres
September - 2023
https://github.com/Esquirio/Arduino-u-blox-NINA-W10

Distributed as-is; no warranty is given.

Description:

**********   Server example   **************

******************************************************************************/
#include "Arduino.h"
#include <DHT11.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

/******************     NINA-W10 PIN Definition      ************************************
LED_GREEN = 33;
LED_RED   = 23;
LED_BLUE  = 21;
SW1       = 33;
SW2       = 27;

TX        = 1;
RX        = 3;

SDA       = 12;
SCL       = 13;
***************************************************************************************/

// Defines
#define DIGITAL_PORT 26

// Functions Prototype
void setupBLE   (void);
void readSensor (void);

// BLE pointers (Global)
BLEServer           *pServer = NULL;
BLEAdvertising      *pAdvertising;
BLECharacteristic   *tCharacteristic = NULL;  // Temperature characteristic
BLECharacteristic   *hCharacteristic = NULL;  // Humidity characteristic
BLEDescriptor       *tDescr;                  // Temperature description
BLEDescriptor       *hDescr;                  // Humidity description

bool deviceConnected = false;
bool oldDeviceConnected = false;

// Create an instance of the DHT11 class.
// - For NINA-W10 (ESP32): Connect the sensor to pin GPIO26 or 26.
DHT11 dht11(DIGITAL_PORT);

// See the following for generating UUIDs: https://www.uuidgenerator.net/
// The remote service we wish to connect to.
static BLEUUID serviceUUID("181A");
// The characteristic of data we will provide
static BLEUUID tcharUUID("2A6E");
static BLEUUID hcharUUID("2A6F");

// This device name
const char charNAME[] = "NINA-W10_Server";

// Declare Global variables
// need to truncate variables to uint16 as defined per BLE
uint16_t t16 = 0;
uint16_t h16 = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      digitalWrite(LED_RED,   HIGH);
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_BLUE,  LOW );
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      digitalWrite(LED_RED,   HIGH);
      digitalWrite(LED_GREEN, LOW );
      digitalWrite(LED_BLUE,  HIGH);
      deviceConnected = false;
    }
};

void setup() {
  pinMode(LED_RED,   OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE,  OUTPUT);
  digitalWrite(LED_RED,   HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE,  HIGH);

  // Start the serial communication
  Serial.begin(115200);
  // Setup the BLE
  setupBLE();
}

void loop() {
  // notify changed value
  if (deviceConnected) {
    
    readSensor();
    tCharacteristic->setValue(t16);
    hCharacteristic->setValue(h16);
    delay(1000);
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

void setupBLE(void){
  // Create the BLE Device
  BLEDevice::init(charNAME);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(serviceUUID);

  // Create a BLE Characteristic
  tCharacteristic = pService->createCharacteristic(
                      tcharUUID,
                      BLECharacteristic::PROPERTY_READ);
  hCharacteristic = pService->createCharacteristic(
                      hcharUUID,
                      BLECharacteristic::PROPERTY_READ);

  // Create a BLE Descriptor
  tDescr = new BLEDescriptor((uint16_t)0x2901);
  tDescr->setValue("Temperature");
  tCharacteristic->addDescriptor(tDescr);

  hDescr = new BLEDescriptor((uint16_t)0x2901);
  hDescr->setValue("Humidity");
  hCharacteristic->addDescriptor(hDescr);
 
  readSensor();
  tCharacteristic->setValue(t16);
  hCharacteristic->setValue(h16);
  // Start the service
  pService->start();

  // Start advertising
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();

  Serial.println("Characteristic defined! Now you can read it in your phone!");
  
}

void readSensor (void){
  // Attempt to read the temperature and humidity values from the DHT11 sensor.
  int temp = dht11.readTemperature();
  delay(50); // Recommended for ESP32 MCUs
  int humi = dht11.readHumidity();

  // need to truncate variables to uint16 as defined per BLE
  t16 = temp * 100;
  h16 = humi * 100;

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

  tCharacteristic->setValue( t16 );
  hCharacteristic->setValue( h16 );
}

