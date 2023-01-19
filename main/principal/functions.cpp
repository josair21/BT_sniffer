#include "BLEDevice.h"
#include "BLEAddress.h"
#include <Arduino.h>
#include "functions.h"
String devices[] = {" ", " ", " ", " ", " "};
String deviceToConnect = " ";
int iDevice = 0;
bool isDuplicated = false, isDone = false;
bool isConnected = false, isFound = false;
BLEAdvertisedDevice* myDevice;
BLEScan* pBLEScan;
BLEClient*  pClient;
static BLEUUID serviceUUID("4aeab230-9aa6-4ca8-9329-1154c1d88d51");
static BLEUUID    charUUID_read("4aeab330-9aa6-4ca8-9329-1154c1d88d51"), charUUID_write("4aeab331-9aa6-4ca8-9329-1154c1d88d51");
static BLERemoteCharacteristic* pRemoteCharacteristic_read;
static BLERemoteCharacteristic* pRemoteCharacteristic_write;
BLERemoteService* pRemoteService;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    if(pData[0] == 0x00 || pData[0] == 0x02){
      for (int i = 0; i < length; i++){
        if(i == 0){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
        }
        else if(i >= 7){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
        }
        else if(i%2 == 1){
          Serial.print(pData[i]*0x100 + pData[i+1], DEC);
          Serial.print(" ");
        }
      }
    }
    else if(pData[0] == 0x01){
      for (int i = 0; i < length; i++){
        if(i == 0){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
        }
        else if(i == length - 1 &&  i%2 != 0){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
        }
        else if(i%2 == 1){
          Serial.print(pData[i]*0x100 + pData[i+1], DEC);
          Serial.print(" ");
        }
      }
    }
    else if(pData[0] == 0x04 && pData[1] == 0x00){
      isDone = true;
      }
    else{
      for (int i = 0; i < length; i++){
        Serial.print(pData[i], DEC);
        Serial.print(" ");
      }
    }
    Serial.println("");
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    isConnected = true;
  }

  void onDisconnect(BLEClient* pclient) {
  }
};
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {  
    devices[iDevice] = advertisedDevice.getName().c_str();
    if(deviceToConnect == devices[iDevice] && deviceToConnect != " "){
      pBLEScan->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      isFound = true;
    }
    for(int i=0; i<5; i++){
      if((devices[iDevice] == devices [i] && iDevice != i) || devices[iDevice] == "" || devices[iDevice] == " "){
        isDuplicated = true;
        devices[iDevice] = " ";
        break;
      }
    }
    if(isDuplicated == false) iDevice++;
    if(iDevice >= 5 ) iDevice = 0;
    isDuplicated = false;
    
  }
};  
void begin_ble(){
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(399);
  pBLEScan->setWindow(299);
  pBLEScan->setActiveScan(true);
}
                               // MyAdvertisedDeviceCallbacks
void clear_ble(){
  for(int i=0; i<5; i++) devices[i] = " ";
  iDevice = 0;
}
void search_ble(){
  pBLEScan->start(1, false);
}
///////////////////////////////
bool connect_ble(int a) {
  if(a > 3 && deviceToConnect[0] == 'C' && deviceToConnect[1] == 'C'){
    pBLEScan->start(1, false);
    if(isFound){
      pClient = BLEDevice::createClient();
      pClient->setClientCallbacks(new MyClientCallback());
      pClient->connect(myDevice);  
      pRemoteService = pClient->getService(serviceUUID);
      pRemoteCharacteristic_read = pRemoteService->getCharacteristic(charUUID_read);
      pRemoteCharacteristic_write = pRemoteService->getCharacteristic(charUUID_write);
      if(pRemoteCharacteristic_read->canNotify())
      pRemoteCharacteristic_read->registerForNotify(notifyCallback);
    }
    if(isConnected){
      isConnected = false;
      return true;
    }
  }
  else delay(1000);
  return false;
  
}
void download_ble(){
  pRemoteCharacteristic_write->writeValue({0x80,0x31,0x0b,0xcc});
  //pRemoteCharacteristic_write->writeValue({0x01,0x00});
  pRemoteCharacteristic_write->writeValue({0x80,0x31,0x0a,0xcc});
  
}
void disconnect_ble(){
  pClient->disconnect();  
}
