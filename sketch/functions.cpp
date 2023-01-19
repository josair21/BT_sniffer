#include "BLEDevice.h"
#include "BLEAddress.h"
#include <Arduino.h>
#include "functions.h"
String devices[] = {" ", " ", " ", " ", " "};

int iDevice = 0;
static BLEAdvertisedDevice* myDevice;
bool isDuplicated = false;
BLEScan* pBLEScan;
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {  
    devices[iDevice] = advertisedDevice.getName().c_str();
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
