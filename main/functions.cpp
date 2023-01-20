#include "BLEDevice.h"
#include "BLEAddress.h"
#include <Arduino.h>
#include "functions.h"
#include <SPI.h>
#include <SD.h>
File myFile;
char Aux[999] = " ";
const int CS = 5;
int dia, mes, ano, hora, minuto;
int hora_intervalo, minuto_intervalo;
float humedad, temperatura;
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
    if(pData[0] == 0x00){
      dia = pData[8];
      mes = pData[7];
      ano = pData[5]*0x100+pData[6];
      hora = pData[9];
      minuto = pData[10];
      humedad = (pData[1]*0x100+pData[2])/10.0;
      temperatura = (pData[3]*0x100+pData[4])/10.0;
      sprintf(Aux,"%02d-%02d-%02d %02d:%02d,%.1f,%.1f", dia, mes, ano, hora, minuto, humedad, temperatura);
    }
    else if(pData[0] == 0x01){
      for(int i=0; i<(int)((length-1)/4); i++){
          if(minuto - minuto_intervalo < 0 ){
            minuto = minuto - minuto_intervalo + 60;
            hora--;
          }
          else minuto = minuto - minuto_intervalo;
          if(hora - hora_intervalo < 0){
            hora = hora - hora_intervalo + 24;
            dia--; 
          }
          
          dia = pData[8];
          mes = pData[7];
          ano = pData[5]*0x100+pData[6];
          hora = pData[9];
          minuto = pData[10];
          humedad = (pData[1]*0x100+pData[2])/10.0;
          temperatura = (pData[3]*0x100+pData[4])/10.0;
          sprintf(Aux," %02d-%02d-%02d %02d:%02d,%.1f,%.1f", dia, mes, ano, hora, minuto, humedad, temperatura);
        }
      }
    }
    else if(pData[0] == 0x04 && pData[1] == 0x00){
      isDone = true;
      }
    
    /*if(pData[0] == 0x00 || pData[0] == 0x02){
      for (int i = 0; i < length; i++){
        if(i == 0){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
          myFile.print(pData[i], DEC);
          myFile.print(" ");
        }
        else if(i >= 7){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
          myFile.print(pData[i], DEC);
          myFile.print(" ");
        }
        else if(i%2 == 1){
          Serial.print(pData[i]*0x100 + pData[i+1], DEC);
          Serial.print(" ");
          myFile.print(pData[i]*0x100 + pData[i+1], DEC);
          myFile.print(" ");
        }
      }
    }
    else if(pData[0] == 0x01){
      for (int i = 0; i < length; i++){
        if(i == 0){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
          myFile.print(pData[i], DEC);
          myFile.print(" ");
        }
        else if(i == length - 1 &&  i%2 != 0){
          Serial.print(pData[i], DEC);
          Serial.print(" ");
          myFile.print(pData[i], DEC);
          myFile.print(" ");
        }
        else if(i%2 == 1){
          Serial.print(pData[i]*0x100 + pData[i+1], DEC);
          Serial.print(" ");
          myFile.print(pData[i]*0x100 + pData[i+1], DEC);
          myFile.print(" ");
        }
      }
    }
    else if(pData[0] == 0x04 && pData[1] == 0x00){
      for (int i = 0; i < length; i++){
        Serial.print(pData[i], DEC);
        Serial.print(" ");
        myFile.print(pData[i], DEC);
        myFile.print(" ");
      }
      isDone = true;
      }
    else{
      for (int i = 0; i < length; i++){
        Serial.print(pData[i], DEC);
        Serial.print(" ");
        myFile.print(pData[i], DEC);
        myFile.print(" ");
      }
    }
    Serial.println();
    myFile.println();
    */
    
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
    }
    if(isConnected){
      isConnected = false;
      return true;
    }
  }
  else delay(1000);
  return false;
  
}
bool download_ble(){
  std::string Data = pRemoteCharacteristic_write->readValue();
  if(!SD.begin(CS)){
    return false;
  }
  myFile = SD.open("/test.csv", FILE_WRITE);
  hora_intervalo = Data[5];
  minuto_intervalo = Data[6];
  sprintf(Aux, "Intervalo de registro: %02d h %02d m\nAlarmas:\n  Humedad minima: %.1f\n  Humedad maxima: %.1f\n", hora_intervalo, minuto_intervalo, (Data[7]*0x100+Data[8])/10.0, (Data[9]*0x100+Data[10])/10.0);
  Serial.print(Aux);
  //myFile.print(Aux);
  sprintf(Aux, "  Humedad minima: %.1f\n  Humedad maxima: %.1f\n",(Data[11]*0x100+Data[12])/10.0, (Data[13]*0x100+Data[14])/10.0);
  Serial.print(Aux);
  sprintf(Aux, "time,HR,temp\n");
  Serial.print(Aux);
  myFile.print(Aux);
  //myFile.print(Aux);
  /*for(int i=0; i<Data.length(); i++){
    if(0 <= i && i <5){
      Serial.print(Data[i], HEX);  
      Serial.print(" ");
      myFile.print(Data[i], HEX);
      myFile.print(" ");
    }
    else if(i == 5){
      sprintf(Aux, "Intervalo %02u horas %02u minutos\n", Data[i], Data[i+1]);
      Serial.print(Aux);
      myFile.print(Aux);
    }
    else if(i == 7){ 
      sprintf(Aux, "Humedad minima: %.1f HR\n", (Data[i]*0x100+Data[i+1])/10;
      Serial.print(Aux);
      myFile.print(Aux);
      Serial.print(Data[i]*0x100+Data[i+1], DEC);
      Serial.print(" ");
      myFile.print(Data[i]*0x100+Data[i+1], DEC);
      myFile.print(" ");
    }
    else if(i == 9){ 
      sprintf(Aux, "Intervalo %02u horas %02u minutos\n", Data[i], Data[i+1]);
      Serial.print(Aux);
      myFile.print(Aux);
      Serial.print(Data[i]*0x100+Data[i+1], DEC);
      Serial.print(" ");
      myFile.print(Data[i]*0x100+Data[i+1], DEC);
      myFile.print(" ");
    }
  }*/
  pRemoteCharacteristic_read->registerForNotify(notifyCallback);
  pRemoteCharacteristic_write->writeValue({0x80,0x31,0x0b,0xcc});
  pRemoteCharacteristic_write->writeValue({0x80,0x31,0x0a,0xcc});
  return true;
  }
void disconnect_ble(){
  myFile.close();
  pClient->disconnect();  
}
