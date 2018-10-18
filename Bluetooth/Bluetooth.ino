#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

int LED = 23;

BLECharacteristic *characteristicTX;

bool deviceConnected = false;

#define SERVICE_UUID "87167e23-7448-458a-a922-3816d5b1f2c5"
#define CHARACTERISTIC_UUID_RX "b61e1874-ac79-4424-bd0d-59b7252d9afc"
#define CHARACTERISTIC_UUID_TX "86bafaa1-274e-4e97-9e69-39d3eb9ed701"


class CharacteristicCallbacks: public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic *characteristic){
    std::string rxValue = characteristic->getValue();
    if (rxValue.length() > 0){
      for (int i = 0; i < rxValue.length(); i++){
        Serial.print(rxValue[i]);
      }
      Serial.println();
      if (rxValue.find("1") != -1){
        Serial.println("LED aceso");
        digitalWrite(LED, HIGH);
      }
      else if (rxValue.find("0") != -1){
        Serial.println("LED apagado");
        digitalWrite(LED, LOW);
      }
    }
  }
};

class ServerCallbacks: public BLEServerCallbacks{
  void onConnect(BLEServer *pServer){
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer){
    deviceConnected = false;
  };
};

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  
  BLEDevice::init("ESP32-BLE");
  BLEServer *server = BLEDevice::createServer();
  BLEService *service = server->createService(SERVICE_UUID);
  server->setCallbacks(new ServerCallbacks());

  characteristicTX = service->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                   );
  characteristicTX->addDescriptor(new BLE2902());

  BLECharacteristic *characteristic = service->createCharacteristic(
                                      CHARACTERISTIC_UUID_RX,
                                      BLECharacteristic::PROPERTY_WRITE
                                     );
  //
  characteristic->setCallbacks(new CharacteristicCallbacks());
  service->start();
  server->getAdvertising()->start();

  Serial.println("Aguardando conexão");
  while(!deviceConnected){
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Conectado");
}

void loop() {
  const char *frase = "Conectado";
  
  if (deviceConnected){
    characteristicTX->setValue(frase);
    characteristicTX->notify();
  }
  delay(30000);
}
