#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <DHT.h>
#include <Arduino.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "3778499a-3453-11ed-a261-0242ac120002"
#define CHARACTERISTIC_UUID "37785020-3453-11ed-a261-0242ac120002"


#define DHTTYPE DHT22   //Tipo di sensore che stiamo utilizzando (DHT22)
#define DHTPIN 4   //Pin a cui è connesso il sensore

#define REFRESH 5000 // Intervallo di tempo tra un refresh e l'altro
#define DELAY 2000 // Intervallo di tempo tra un refresh e l'altro

DHT dht(DHTPIN, DHTTYPE); //Inizializza oggetto chiamato "dht", parametri: pin a cui è connesso il sensore, tipo di dht 11/22

unsigned long myTime; //Variabile per salvare il tempo passato dall'inzinio dell'esecuzione del programma

float hum = 0;  //Variabile in cui verrà inserita la % di umidità
float newHum = 0;
float averageHum = 0;

float temp = 0; //Variabile in cui verrà inserita la temperatura
float newTemp = 0;
float averageTemp = 0;

int detectionCount = 0;

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;
BLEAdvertising *pAdvertising;


void setup()
{
  Serial.begin(115200);
  dht.begin();
  Serial.println("Starting BLE Server!");

  BLEDevice::init("ESP32-DHTServer");
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );


  pCharacteristic->setValue("DHTValue");
  pService->start();
  pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();
  Serial.println("Characteristic defined! Now you can read it in the Client!");
}

void loop()
{
  myTime = millis();

  if (myTime%REFRESH == 0) { //Se il tempo passato dall'inizio dell'esecuzione del programma è multiplo di REFRESH
    newHum = dht.readHumidity();
    newTemp = dht.readTemperature();
    Serial.print("Umidità rilevata: ");
    Serial.println(newHum);
    Serial.print("Temperatura rilevata: ");
    Serial.println(newTemp);
    detectionCount += 1;
    hum += newHum;
    temp += newTemp;
    averageHum = hum / detectionCount;
    averageTemp = temp / detectionCount;
    std::string humString = std::to_string(averageHum);
    std::string tempString = std::to_string(averageTemp);
    std::string res = "(" + humString + ")-(" + tempString + ")";
    pCharacteristic->setValue(res);
    Serial.println(res.c_str());
  }

  if (myTime%DELAY == 0) { //Se il tempo passato dall'inizio dell'esecuzione del programma è multiplo di REFRESH
    if(pServer->getConnectedCount() == 0){
      //Serial.println("Nessun dispositivo connesso inizio l'advertising");
      pAdvertising->start();
    }else{
      //pCharacteristic->setValue(std::to_string(averageLight));
      hum = 0;
      temp = 0;
      detectionCount = 0;
      std::string value = pCharacteristic->getValue();
      Serial.print("The new characteristic value is: ");
      Serial.println(value.c_str());
    }
  }
}